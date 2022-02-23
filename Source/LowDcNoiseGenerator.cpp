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

float LowDcNoiseGenerator::getNextSample() {
    auto outSample = this->buffer.getSample(0, this->read);

    auto newSample = this->random.nextFloat() * 2.f - 1.f;
    this->runningTotal += newSample;
    this->buffer.setSample(0, read, newSample);

    ++this->read;

    // If the buffer is full...
    if (this->read == BUFFER_LENGTH) {
        // ...find DC offset...
        auto mean = this->runningTotal / (float) BUFFER_LENGTH;
        // ...and subtract it from each sample in the buffer.
        for (int i = 0; i < BUFFER_LENGTH; ++i) {
            this->buffer.setSample(0, i, this->buffer.getSample(0, i) - mean);
        }
        // Reset the counters.
        this->runningTotal = 0.f;
        this->read = 0;
    }

    return outSample;
}
