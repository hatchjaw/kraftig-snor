/*
  ==============================================================================

    LowDcNoiseGenerator.h
    Created: 22 Feb 2022 7:40:19pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class LowDcNoiseGenerator {
public:
    enum NoiseMode {
        NOISE,
        BIPOLAR_IMPULSES
    };

    LowDcNoiseGenerator();

    float getNextSample();

    void setMode(NoiseMode newMode);

private:
    const int BUFFER_LENGTH{64};
    NoiseMode mode{BIPOLAR_IMPULSES};
    juce::AudioBuffer<float> buffer{1, BUFFER_LENGTH};
    juce::Random random{juce::Time::currentTimeMillis()};
    float runningTotal{0.f};
    int read{0};
};
