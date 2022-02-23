/*
  ==============================================================================

    KsResonator.h
    Created: 21 Feb 2022 5:20:00pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LowDcNoiseGenerator.h"
#include "AllpassFilter.h"

class KsResonator {
public:
    void prepareToPlay(juce::dsp::ProcessSpec &);

    void addSympatheticResonator();

    void updateSympatheticResonators(double sampleRate, float freq1, float amount1, float freq2, float amount2);

    void renderNextBlock(juce::dsp::AudioBlock<float> &, int startSample, int numSamples);

    void setupNote(double sampleRate, double frequency, float noteAmplitude);

    void stopNote();

    bool isActive();

    void setExcitationEnvelope(juce::ADSR::Parameters &newParameters);

    void updateMutePrimaryResonator(bool shouldMute);

    void updateStretchFactor(float newDamping);

    void updatePrimaryInharmonicity(float allpassGain, int allpassOrder);

    void enableExcitationEnvelope(bool enable);

    void setExcitationMode(LowDcNoiseGenerator::NoiseMode mode);

private:
    /**
     * A scaling value for the excitation to the primary resonator.
     */
    const float EXCITATION_SCALAR{1.f};
    /**
     * A scaling value for the excitation to the sympathetic resonators.
     */
    const float SYMPATHETIC_SCALAR{.25f};

    void initDelayLine(double sampleRate, double frequencyToUse);

    float computeNextSample(float excitationSample);

    float getExcitationSample();

    float envelopeExcitationSample(float excitationSample);

    juce::ADSR envelope;

    LowDcNoiseGenerator excitation;
    bool useExcitationEnvelope{false};
    juce::ADSR excitationEnvelope;
    uint excitationCounter;

    float stretchFactor{.5f};
    juce::Random stretchRandom{juce::Time::currentTimeMillis()};

    double fractionalDelay{0.0};
    uint delayLineLength{0};
    juce::AudioBuffer<float> delayLine;
    int delayLineWriteIndex{0};

    juce::OwnedArray<KsResonator> sympatheticResonators;
    double frequency{0.0};

    bool isSympathetic{false};

    float amplitude{0.f};
    bool mutePrimary{false};

    AllpassFilter allpass;
};
