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
    for (auto &sr: this->sympatheticResonators) {
        sr->prepareToPlay(spec);
    }
}

void KsResonator::renderNextBlock(juce::dsp::AudioBlock<float> &block, int startSample, int numSamples) {
    while (--numSamples >= 0) {
        auto excitationSample = this->getExcitationSample();

//        auto currentSample = Utils::clamp(this->computeNextSample(excitationSample), 1.0f);
        auto currentSample = this->computeNextSample(excitationSample);

        for (auto i = (int) block.getNumChannels(); --i >= 0;) {
            block.addSample((int) i, startSample, currentSample);
        }

        ++startSample;
    }
}

float KsResonator::getExcitationSample() {
    float sample;

    if (useExcitationEnvelope && this->excitationEnvelope.isActive()) {
        sample = this->excitation.getNextSample() * this->excitationEnvelope.getNextSample();
    } else if (!useExcitationEnvelope && this->excitationCounter > 0) {
        sample = this->excitation.getNextSample();
        --this->excitationCounter;
    } else {
        sample = 0.f;
    }

    sample *= EXCITATION_SCALAR;

    return sample;
}

float KsResonator::computeNextSample(float excitationSample) {
    auto delayLineReadIndex = Utils::ksModulo(this->delayLineWriteIndex - this->fractionalDelay, this->delayLineLength);
    double integralPart;
    auto weight = modf(delayLineReadIndex, &integralPart);
    auto currentIndex = static_cast<int>(integralPart);

    auto curr = delayLine.getSample(0, currentIndex);
    auto prev = delayLine.getSample(0, Utils::wrapIndex(currentIndex - 1, static_cast<int>(this->delayLineLength)));
    auto next = delayLine.getSample(0, Utils::wrapIndex(currentIndex + 1, static_cast<int>(this->delayLineLength)));

    // y[n-L]
    auto yn_L = (1. - weight) * curr + weight * next;
    // y[n-(L+1)]
    auto yn_L_1 = (1. - weight) * prev + weight * curr;

    // Karplus-Strong
    // y_{ks}[n] = x[n] + .5*y[n-L] + .5*y[n-(L+1)]
    // TODO: decay stretching/shortening
    auto sample = static_cast<float>(
            // Scale the excitation by the input amplitude/velocity
            this->amplitude * excitationSample +
            (1. - this->damping) * yn_L +
            this->damping * yn_L_1
    );

    // Inharmonicity
    sample = this->allpass.processSample(sample);

    // Apply envelope. Damp primary resonator on note-off (but let sympathetic resonators ring).
    sample *= this->envelope.getNextSample();

    // Update the delay line.
    this->delayLine.setSample(0, this->delayLineWriteIndex, sample);
    ++this->delayLineWriteIndex;
    this->delayLineWriteIndex %= (int) this->delayLineLength;

    if (!this->isSympathetic) {
        // Handle sympathetic resonators. Basic, one-way relationship, i.e. sympathetic resonators don't feed into each
        // other, nor the primary resonator.
        auto sympathetic = 0.f;
        for (auto &sr: this->sympatheticResonators) {
            // Yes, I know amplitude is being used within the recursive call; *this* use of amplitude means it's
            // possible to fade the sympathetic resonators in and out parametrically too.
            sympathetic += sr->amplitude * sr->computeNextSample(sample * SYMPATHETIC_SCALAR);
        }

        sample += sympathetic;

        if (this->mutePrimary) {
            return sympathetic;
        }
    }

    return sample;
}

void KsResonator::addSympatheticResonator() {
    auto resonator = new KsResonator;
    resonator->isSympathetic = true;
    this->sympatheticResonators.add(resonator);
}

void KsResonator::setupNote(double sampleRate, double frequency, float noteAmplitude) {
    this->amplitude = noteAmplitude;
    this->envelope.noteOn();

    this->initDelayLine(sampleRate, frequency);

    // Set up excitation
    if (this->useExcitationEnvelope) {
        this->excitationEnvelope.noteOn();
    } else {
        this->excitationCounter = static_cast<uint>(ceil(sampleRate / frequency));
    }

    for (auto &sr: this->sympatheticResonators) {
        sr->delayLine.clear();
        sr->envelope.noteOn();
        sr->excitationEnvelope.noteOn();
    }
}

void KsResonator::initDelayLine(double sampleRate, double frequencyToUse) {
    this->frequency = frequencyToUse;
    this->fractionalDelay = sampleRate / frequency;
    this->delayLineLength = static_cast<uint>(ceil(this->fractionalDelay) + 1);
    this->delayLine.setSize(1, static_cast<int>(this->delayLineLength), true, true, true);
    this->delayLineWriteIndex = 0;
}

void KsResonator::stopNote() {
    if (this->useExcitationEnvelope) this->excitationEnvelope.noteOff();
    this->envelope.noteOff();
}

bool KsResonator::isActive() {
    return this->envelope.isActive() ||
           (this->useExcitationEnvelope && this->excitationEnvelope.isActive()) ||
           (this->excitationCounter > 0);
}

void KsResonator::setExcitationEnvelope(juce::ADSR::Parameters &newParameters) {
    this->excitationEnvelope.setParameters(newParameters);
}

void KsResonator::updateSympatheticResonators(
        double sampleRate,
        float freq1,
        float amount1,
        float freq2,
        float amount2
) {
    if (this->sympatheticResonators[0]->frequency != freq1) {
        this->sympatheticResonators[0]->initDelayLine(sampleRate, freq1);
    }
    this->sympatheticResonators[0]->amplitude = amount1;
    if (this->sympatheticResonators[1]->frequency != freq2) {
        this->sympatheticResonators[1]->initDelayLine(sampleRate, freq2);
    }
    this->sympatheticResonators[1]->amplitude = amount2;
}

void KsResonator::updateMutePrimaryResonator(bool shouldMute) {
    this->mutePrimary = shouldMute;
}

void KsResonator::updateDamping(float newDamping) {
    this->damping = newDamping;
}

void KsResonator::updatePrimaryInharmonicity(float allpassGain, int allpassOrder) {
    this->allpass.setGain(allpassGain);
    this->allpass.setOrder(static_cast<uint>(allpassOrder));
}

void KsResonator::setExcitationMode(LowDcNoiseGenerator::NoiseMode mode) {
    this->excitation.setMode(mode);
}

void KsResonator::enableExcitationEnvelope(bool enable) {
    this->useExcitationEnvelope = enable;
    if (enable) {
        this->excitationCounter = 0;
    }
}
