/*
  ==============================================================================

    LowDcNoiseGenerator.cpp
    Created: 22 Feb 2022 7:40:19pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#include "LowDcNoiseGenerator.h"

LowDcNoiseGenerator::LowDcNoiseGenerator() {
    // Pre-fill the buffer.
    for (int i = 0; i < BUFFER_LENGTH; ++i) {
        this->getNextSample();
    }
}

/**
 * Nice idea, but doesn't stop the resonator floating away sometimes.
 * @return
 */
float LowDcNoiseGenerator::getNextSample() {
    auto outSample = this->buffer.getSample(0, this->read);

    auto nextFloat = this->random.nextFloat();
    auto newSample = 0.f;
    switch (this->mode) {
        case BIPOLAR_IMPULSES:
            newSample = nextFloat > .5f ? 1.f : -1.f;
            break;
        case NOISE:
            newSample = nextFloat * 2.f - 1.f;
            break;
        default:
            jassertfalse;
    }

    this->runningTotal += newSample;
    this->buffer.setSample(0, read, newSample);

    ++this->read;

    // If the buffer is full...
    if (this->read == BUFFER_LENGTH) {
//      if (this->mode == NOISE) {
        // ...find DC offset...
        auto mean = this->runningTotal / (float) BUFFER_LENGTH;
        // ...and subtract it from each sample in the buffer.
        for (int i = 0; i < BUFFER_LENGTH; ++i) {
            this->buffer.setSample(0, i, this->buffer.getSample(0, i) - mean);
        }
//      }

        // Reset the counters.
        this->runningTotal = 0.f;
        this->read = 0;
    }

    return outSample;
}

void LowDcNoiseGenerator::setMode(LowDcNoiseGenerator::NoiseMode newMode) {
    this->mode = newMode;
}
