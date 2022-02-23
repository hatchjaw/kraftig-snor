/*
  ==============================================================================

    AllpassFilter.cpp
    Created: 23 Feb 2022 2:29:20pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#include "AllpassFilter.h"
#include "Utils.h"

void AllpassFilter::setGain(float newGain) {
    this->gain = newGain;
}

void AllpassFilter::setOrder(uint newOrder) {
    if (newOrder != this->order) {
        this->order = newOrder;
        if (newOrder > 0) {
            this->buffer.setSize(2, static_cast<int>(newOrder), true, true, true);
            this->writeIndex %= this->order;
        }
    }
}

float AllpassFilter::processSample(float inputSample) {
    float outSample;

    if (this->order == 0) {
        outSample = inputSample;
        this->writeIndex = 0;
    } else {
        auto readIndex = static_cast<uint>(Utils::ksModulo(this->writeIndex - this->order, this->order));

        // y[n] = gx[n] + x[n-N] - gy[n-N]
        outSample = this->gain * inputSample +
                         this->buffer.getSample(0, static_cast<int>(readIndex)) -
                         this->gain * this->buffer.getSample(1, static_cast<int>(readIndex));

        this->buffer.setSample(0, static_cast<int>(this->writeIndex), inputSample);
        this->buffer.setSample(1, static_cast<int>(this->writeIndex), outSample);
        ++this->writeIndex;
        this->writeIndex %= this->order;
    }

    return outSample;
}
