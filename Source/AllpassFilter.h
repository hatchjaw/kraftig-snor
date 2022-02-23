/*
  ==============================================================================

    AllpassFilter.h
    Created: 23 Feb 2022 2:29:20pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class AllpassFilter {
public:
    void setGain(float newGain);

    void setOrder(uint newOrder);

    float processSample(float inputSample);

private:
    float gain{0.f};
    uint order{0};
    // Use channel 0 for feedforward buffer, channel 1 for feedback buffer.
    juce::AudioBuffer<float> buffer{2, 0};
    uint writeIndex{0};
};