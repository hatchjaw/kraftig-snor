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

    // maybe don't need this.
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
    // TODO: Should stop as if due to damping
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

//    this->resonator->prepareToPlay(spec);

    this->isPrepared = true;
}

