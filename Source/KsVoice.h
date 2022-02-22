/*
  ==============================================================================

    KsVoice.h
    Created: 21 Feb 2022 3:10:50pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "KsResonator.h"

class KsVoice : public juce::SynthesiserVoice {
public:
    ~KsVoice() override;

    bool canPlaySound(juce::SynthesiserSound *sound) override;

    void startNote(int midiNoteNumber,
                   float velocity,
                   juce::SynthesiserSound *sound,
                   int currentPitchWheelPosition) override;

    void stopNote(float velocity, bool allowTailOff) override;

    void controllerMoved(int controllerNumber, int newControllerValue) override;

    void pitchWheelMoved(int newPitchWheelValue) override;

    void prepareToPlay(double sampleRate, int samplesPerBlock, int numOutputChannels);

    void renderNextBlock(juce::AudioBuffer<float> &outputBuffer,
                         int startSample,
                         int numSamples) override;

    void setExcitationEnvelope(juce::ADSR::Parameters &newParameters);

    void addSympatheticResonator();

    void updateSympatheticResonators(float freq1, float amount1, float freq2, float amount2);

    void updateMutePrimary(bool shouldMute);

private:
    bool isPrepared;
    juce::AudioBuffer<float> buffer;
    KsResonator resonator;
};