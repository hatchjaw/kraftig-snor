/*
  ==============================================================================

    KsResonator.h
    Created: 21 Feb 2022 5:20:00pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class KsResonator {
public:
    void prepareToPlay(juce::dsp::ProcessSpec &);

    void addSympatheticResonator();

    void renderNextBlock(juce::dsp::AudioBlock<float> &, int startSample, int numSamples);

    float computeNextSample();

    void setupNote(double sampleRate, double frequency, float noteAmplitude);

    void stopNote();

    bool isActive();

    void setExcitationEnvelope(juce::ADSR::Parameters &newParameters);

private:
    float getExcitationSample();

    juce::ADSR envelope;

    juce::Random excitation;
    juce::ADSR excitationEnvelope;

    double fractionalDelay{0.0};
    uint delayLineLength{0};
    juce::AudioBuffer<float> delayLine;
    int delayLineWriteIndex{0};

    juce::OwnedArray<KsResonator> sympatheticResonators;

    float amplitude{0.f};
};
