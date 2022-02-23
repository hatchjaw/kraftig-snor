/*
  ==============================================================================

    Utils.h
    Created: 21 Feb 2022 9:42:26pm
    Author:  Tommy Rushton

  ==============================================================================
*/

#pragma once

#include <cmath>

class Utils {
public:
    /**
     * Calculate the modulo of a signed double against an unsigned integer; works like MATLAB for negative numbers.
     * @param a Index to check against the table size
     * @param b Size of the table
     * @return
     */
    static double ksModulo(double a, uint b) {
        return fmod(b + fmod(a, b), b);
    }

    static int wrapIndex(int index, int length) {
        if (index >= length) {
            index = 0;
        } else if (index < 0) {
            index = length - 1;
        }
        return index;
    }

    static float clamp(float input, float threshold) {
        if (input > threshold) {
            input = threshold;
        } else if (input < -threshold) {
            input = -threshold;
        }
        return input;
    }
};
