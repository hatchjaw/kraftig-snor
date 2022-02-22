/*
  ==============================================================================

    LowDcNoiseGenerator.h
    Created: 22 Feb 2022 7:40:19pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/**
 * Nice idea, but doesn't stop the resonator floating away sometimes.
 */
class LowDcNoiseGenerator {
public:

    LowDcNoiseGenerator();

    float getNextSample();

private:
    const int BUFFER_LENGTH{8};
    juce::AudioBuffer<float> buffer{1, BUFFER_LENGTH};
    juce::Random random{juce::Time::currentTimeMillis()};
    float runningTotal{0.f};
    int read{0};
};