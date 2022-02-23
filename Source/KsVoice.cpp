/*
  ==============================================================================

    KsVoice.cpp
    Created: 21 Feb 2022 3:10:50pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#include "KsVoice.h"
#include "KsSound.h"

KsVoice::~KsVoice() {

}

bool KsVoice::canPlaySound(juce::SynthesiserSound *sound) {
    return dynamic_cast<KsSound *>(sound) != nullptr;
}

void KsVoice::renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples) {
    jassert(this->isPrepared);

    if (!this->isVoiceActive()) {
        return;
    }

    // MIDI messages can occur at any point during a buffer,
    // so prevent discontinuities by writing to a temp buffer.
    this->buffer.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
    this->buffer.clear();

    juce::dsp::AudioBlock<float> audioBlock{this->buffer};

    this->resonator.renderNextBlock(audioBlock, 0, (int) audioBlock.getNumSamples());

    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
        outputBuffer.addFrom(channel, startSample, this->buffer, channel, 0, numSamples);
    }
}

void KsVoice::startNote(
        int midiNoteNumber,
        float velocity,
        juce::SynthesiserSound *sound,
        int currentPitchWheelPosition
) {
    this->resonator.setupNote(
            getSampleRate(),
            juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber),
            velocity
    );
}

void KsVoice::stopNote(float velocity, bool allowTailOff) {
    this->resonator.stopNote();

    if (!allowTailOff || !this->resonator.isActive()) {
        clearCurrentNote();
    }
}

void KsVoice::controllerMoved(int controllerNumber, int newControllerValue) {

}

void KsVoice::pitchWheelMoved(int newPitchWheelValue) {
    // TODO: handle glissando/portamento
}

void KsVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int numOutputChannels) {
    this->clearCurrentNote();
    juce::dsp::ProcessSpec spec{sampleRate, static_cast<juce::uint32>(samplesPerBlock),
                                static_cast<juce::uint32>(numOutputChannels)};

    this->resonator.prepareToPlay(spec);

    this->isPrepared = true;
}

void KsVoice::setExcitationEnvelope(juce::ADSR::Parameters &newParameters) {
    this->resonator.setExcitationEnvelope(newParameters);
}

void KsVoice::addSympatheticResonator() {
    this->resonator.addSympatheticResonator();
}

void KsVoice::updateSympatheticResonators(float freq1, float amount1, float freq2, float amount2) {
    this->resonator.updateSympatheticResonators(getSampleRate(), freq1, amount1, freq2, amount2);
}

void KsVoice::updateMutePrimary(bool shouldMute) {
    this->resonator.updateMutePrimaryResonator(shouldMute);
}

void KsVoice::updateDamping(float newDamping) {
    this->resonator.updateDamping(newDamping);
}

void KsVoice::updatePrimaryInharmonicity(float allpassGain, int allpassOrder) {
    this->resonator.updatePrimaryInharmonicity(allpassGain, allpassOrder);
}

void KsVoice::enableExcitationEnvelope(bool enable) {
    this->resonator.enableExcitationEnvelope(enable);
}

void KsVoice::setExcitationMode(LowDcNoiseGenerator::NoiseMode mode) {
    this->resonator.setExcitationMode(mode);
}
