/*
  ==============================================================================

    KsResonator.cpp
    Created: 21 Feb 2022 5:20:00pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#include "KsResonator.h"
#include "Utils.h"

void KsResonator::prepareToPlay(juce::dsp::ProcessSpec &spec) {
    this->envelope.setSampleRate(spec.sampleRate);
    this->envelope.setParameters(juce::ADSR::Parameters{0.f, 0.f, 1.f, .5f});

    this->excitationEnvelope.setSampleRate(spec.sampleRate);
    for (auto &sympathetic: this->sympatheticResonators) {
        sympathetic->prepareToPlay(spec);
    }
}

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
    auto delayLineReadIndex = modulo(this->delayLineWriteIndex - this->fractionalDelay, this->delayLineLength);
    double integralPart;
    auto weight = modf(delayLineReadIndex, &integralPart);
    auto currentIndex = static_cast<int>(integralPart);

    auto curr = delayLine.getSample(0, currentIndex);
    auto prev = delayLine.getSample(0, wrapIndex(currentIndex - 1, static_cast<int>(this->delayLineLength)));
    auto next = delayLine.getSample(0, wrapIndex(currentIndex + 1, static_cast<int>(this->delayLineLength)));

    // Karplus-Strong
    // y_{ks}[n] = x[n] + .5*y[n-L] + .5*y[n-(L+1)]
    // Damp primary resonator on note-off, but let sympathetic resonators ring.
    auto sample = this->envelope.getNextSample() * static_cast<float>(
            // Scale the excitation by the input amplitude/velocity
            this->amplitude * this->getExcitationSample() +
            .5f * ((1 - weight) * curr + weight * next) +
            .5f * ((1 - weight) * prev + weight * curr)
    );

    this->delayLine.setSample(0, this->delayLineWriteIndex, sample);
    ++this->delayLineWriteIndex;
    this->delayLineWriteIndex %= this->delayLineLength;

    return sample;
}

void KsResonator::addSympatheticResonator() {
    this->sympatheticResonators.add(new KsResonator);
}

void KsResonator::setupNote(double sampleRate, double frequency, float noteAmplitude) {
    this->amplitude = noteAmplitude;
    this->envelope.noteOn();

    // Set delay line length
    this->fractionalDelay = sampleRate / frequency;
    this->delayLineLength = static_cast<uint>(ceil(this->fractionalDelay) + 1);
    this->delayLine.setSize(1, static_cast<int>(this->delayLineLength), true, true, true);
    this->delayLineWriteIndex = 1;

    // Set up excitation
    this->excitation.setSeed(juce::Time::currentTimeMillis());
    this->excitationEnvelope.noteOn();
}

void KsResonator::stopNote() {
    this->excitationEnvelope.noteOff();
    this->envelope.noteOff();
}

bool KsResonator::isActive() {
    return this->envelope.isActive() || this->excitationEnvelope.isActive();
}

void KsResonator::setExcitationEnvelope(juce::ADSR::Parameters &newParameters) {
    this->excitationEnvelope.setParameters(newParameters);
}

float KsResonator::getExcitationSample() {
    if (this->excitationEnvelope.isActive()) {
        return this->excitationEnvelope.getNextSample() * (this->excitation.nextFloat() * 2.f - 1.f);
    } else {
        return 0.f;
    }
}