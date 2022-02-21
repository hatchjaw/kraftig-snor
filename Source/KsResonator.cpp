/*
  ==============================================================================

    KsResonator.cpp
    Created: 21 Feb 2022 5:20:00pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#include "KsResonator.h"

void KsResonator::renderNextBlock(juce::dsp::AudioBlock<float> &block, int startSample, int numSamples) {
    while (--numSamples >= 0) {
        auto currentSample = this->computeNextSample();

        for (auto i = (int) block.getNumChannels(); --i >= 0;) {
            block.addSample((int) i, startSample, currentSample);
        }

        ++startSample;
    }
}

float KsResonator::computeNextSample() {
    // TODO: fractional delay
    auto delayLinePrev = this->delayLineReadIndex - 1;
    if (delayLinePrev < 0) {
        delayLinePrev = this->delayLine.getNumSamples() - 1;
    }
    auto sample = this->excitation.getSample(0, this->excitationReadIndex) +
                  .5f * (delayLine.getSample(0, this->delayLineReadIndex)) +
                  .5f * (delayLine.getSample(0, delayLinePrev));

    this->excitation.setSample(0, this->excitationReadIndex, 0.f);
    ++this->excitationReadIndex;
    this->excitationReadIndex %= EXCITATION_LENGTH;

    this->delayLine.setSample(0, this->delayLineReadIndex, sample);
    ++this->delayLineReadIndex;
    this->delayLineReadIndex %= this->delayLine.getNumSamples();

    return sample;
}

void KsResonator::addSympatheticResonator(float frequency) {

}

void KsResonator::setupNote(double sampleRate, double frequency, float noteAmplitude) {
    // Set delay line length
    auto delayLineLength = (int) ceil(sampleRate / frequency) + 1;
    this->delayLine.setSize(1, delayLineLength, true, true, true);
    this->delayLineReadIndex = 1;

    // Set up excitation
    this->excitation.clear();
    this->excitation.setSize(1, EXCITATION_LENGTH, false, true, true);

    // TODO: don't do this up-front; generate a new random sample per sample, and use an envelope.
    juce::Random r{juce::Time::currentTimeMillis()};
    for (int i = 0; i < this->excitation.getNumSamples(); ++i) {
        this->excitation.setSample(0, i, r.nextFloat() * 2. - 1.);
    }
    this->excitationReadIndex = 0;
}
