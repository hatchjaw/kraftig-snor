/*
  ==============================================================================

    Helpers.h
    Created: 21 Feb 2022 9:42:26pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#pragma once

#include <cmath>

/**
 * Calculate the modulo of a signed double against an unsigned integer; works like MATLAB for negative numbers.
 * @param a
 * @param b
 * @return
 */
double modulo(double a, uint b) {
    return fmod(b + fmod(a, b), b);
}

int wrapIndex(int index, int length) {
    if (index >= length) {
        index = 0;
    } else if (index < 0) {
        index = length - 1;
    }
    return index;
}