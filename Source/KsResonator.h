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
    void addSympatheticResonator(float frequency);

    void renderNextBlock(juce::dsp::AudioBlock<float> &, int startSample, int numSamples);

    float computeNextSample();

    void setupNote(double sampleRate, double frequency, float noteAmplitude);

private:
    const int EXCITATION_LENGTH{1000};

    juce::AudioBuffer<float> excitation;
    int excitationReadIndex{0};
    juce::AudioBuffer<float> delayLine;
    int delayLineReadIndex{1};
    juce::OwnedArray<KsResonator> sympatheticResonators;
};
