/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "KsSound.h"
#include "KsVoice.h"

//==============================================================================
KraftigSnorAudioProcessor::KraftigSnorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
        : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                                 .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                                 .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
), apvts(*this, nullptr, "Parameters", KraftigSnorAudioProcessor::createParams())
#endif
{
    ksSynth.addSound(new KsSound());

    for (int i = 0; i < NUM_VOICES; ++i) {
        // Create a Karplus-Strong voice.
        auto voice = new KsVoice();
        // Add a couple of sympathetic resonators.
        voice->addSympatheticResonator();
        voice->addSympatheticResonator();
        // Add the voice to the synth.
        ksSynth.addVoice(voice);
    }
}

KraftigSnorAudioProcessor::~KraftigSnorAudioProcessor() {
}

//==============================================================================
const juce::String KraftigSnorAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool KraftigSnorAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool KraftigSnorAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool KraftigSnorAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double KraftigSnorAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int KraftigSnorAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int KraftigSnorAudioProcessor::getCurrentProgram() {
    return 0;
}

void KraftigSnorAudioProcessor::setCurrentProgram(int index) {
}

const juce::String KraftigSnorAudioProcessor::getProgramName(int index) {
    return {};
}

void KraftigSnorAudioProcessor::changeProgramName(int index, const juce::String &newName) {
}

//==============================================================================
void KraftigSnorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    ksSynth.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < ksSynth.getNumVoices(); ++i) {
        if (auto voice = dynamic_cast<KsVoice *>(ksSynth.getVoice(i))) {
            voice->prepareToPlay(sampleRate, samplesPerBlock, this->getTotalNumOutputChannels());
        }
    }
}

void KraftigSnorAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations

bool KraftigSnorAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

#endif

void KraftigSnorAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    auto excitationMode = static_cast<LowDcNoiseGenerator::NoiseMode>(this->apvts.getRawParameterValue("EXCITATION_TYPE")->load());
    auto excitationEnvelopeOn = this->apvts.getRawParameterValue("ENVELOPE_EXCITATION")->load() > .5f;

    auto excitationAttack = this->apvts.getRawParameterValue("EXCITE_A")->load();
    auto excitationDecay = this->apvts.getRawParameterValue("EXCITE_D")->load();
    auto excitationSustain = this->apvts.getRawParameterValue("EXCITE_S")->load();
    auto excitationRelease = this->apvts.getRawParameterValue("EXCITE_R")->load();
    juce::ADSR::Parameters excitationAdsrParams{excitationAttack, excitationDecay, excitationSustain,
                                                excitationRelease};

    auto damping = this->apvts.getRawParameterValue("DAMPING")->load();

    auto inharmonicity1Gain = this->apvts.getRawParameterValue("INHARMONICITY_PRIMARY_GAIN")->load();
    auto inharmonicity1Order = static_cast<int>(this->apvts.getRawParameterValue(
            "INHARMONICITY_PRIMARY_ORDER")->load());

    auto sympatheticOnly = apvts.getRawParameterValue("SYMPATHETIC_ONLY")->load() > .5f;
    auto sympathetic1Freq = this->apvts.getRawParameterValue("SYMPATHETIC_1_FREQ")->load();
    auto sympathetic1Amount = this->apvts.getRawParameterValue("SYMPATHETIC_1_AMOUNT")->load();
    auto sympathetic2Freq = this->apvts.getRawParameterValue("SYMPATHETIC_2_FREQ")->load();
    auto sympathetic2Amount = this->apvts.getRawParameterValue("SYMPATHETIC_2_AMOUNT")->load();

    for (int i = 0; i < ksSynth.getNumVoices(); ++i) {
        if (auto voice = dynamic_cast<KsVoice *>(ksSynth.getVoice(i))) {
            voice->setExcitationMode(excitationMode);
            voice->enableExcitationEnvelope(excitationEnvelopeOn);
            voice->setExcitationEnvelope(excitationAdsrParams);
            voice->updateSympatheticResonators(sympathetic1Freq,
                                               sympathetic1Amount,
                                               sympathetic2Freq,
                                               sympathetic2Amount);
            voice->updateDamping(damping);
            voice->updatePrimaryInharmonicity(inharmonicity1Gain, inharmonicity1Order);
            voice->updateMutePrimary(sympatheticOnly);
        }
    }

    ksSynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool KraftigSnorAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *KraftigSnorAudioProcessor::createEditor() {
//    return new KraftigSnorAudioProcessorEditor(*this);

    // Let JUCE create a generic UI
    auto editor = new juce::GenericAudioProcessorEditor(*this);
    editor->setSize(400, 575);
    return editor;
}

//==============================================================================
void KraftigSnorAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void KraftigSnorAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout KraftigSnorAudioProcessor::createParams() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
            "EXCITATION_TYPE",
            "Excitation type",
            juce::StringArray{"Noise", "Impulses"},
            1
    ));

    params.push_back(std::make_unique<juce::AudioParameterBool>("ENVELOPE_EXCITATION", "Excitation envelope on", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "EXCITE_A",
            "Excitation attack (s)",
            juce::NormalisableRange<float>(0.f, .25f, .001f),
            .01f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "EXCITE_D",
            "Excitation Decay (s)",
            juce::NormalisableRange<float>(0.f, .25f, .001f),
            .1f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "EXCITE_S",
            "Excitation Sustain",
            juce::NormalisableRange<float>(0.f, 1.f, .001f),
            0.f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "EXCITE_R",
            "Excitation Release (s)",
            juce::NormalisableRange<float>(0.f, .5f, .001f),
            0.f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "DAMPING",
            "Damping",
            juce::NormalisableRange<float>(0.f, 1.f, .001f),
            .5f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "INHARMONICITY_PRIMARY_GAIN",
            "Inharmonicity gain",
            juce::NormalisableRange<float>(0.f, .999f, .001f),
            0.f
    ));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
            "INHARMONICITY_PRIMARY_ORDER",
            "Inharmonicity order",
            0, 1000, 0
    ));

    params.push_back(std::make_unique<juce::AudioParameterBool>("SYMPATHETIC_ONLY", "Mute primary resonator", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "SYMPATHETIC_1_FREQ",
            "Sympathetic 1 Frequency",
            juce::NormalisableRange<float>(100.f, 1000.f, .01f),
            261.6256f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "SYMPATHETIC_1_AMOUNT",
            "Sympathetic 1 Amount",
            juce::NormalisableRange<float>(0.f, 1.f, .01f),
            0.f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "SYMPATHETIC_2_FREQ",
            "Sympathetic 2 Frequency",
            juce::NormalisableRange<float>(100.f, 1000.f, .01f),
            392.4383f
    ));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "SYMPATHETIC_2_AMOUNT",
            "Sympathetic 2 Amount",
            juce::NormalisableRange<float>(0.f, 1.f, .01f),
            0.f
    ));

    return {params.begin(), params.end()};
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE

createPluginFilter() {
    return new KraftigSnorAudioProcessor();
}
