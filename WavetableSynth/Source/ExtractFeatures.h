/*
  =============================================================================

    ExtractFeatures.
    Author:  湯谷承

  =============================================================================
*/
#include <juce_dsp/juce_dsp.h>

#pragma onc

#ifndef WAVETABLE_FEATURES_H
#define WAVETABLE_FEATURES_H

#include <vector>
#include <complex>
#include <JuceHeader.h>

std::vector<float> linspace(float start, float end, int num);
float geometricMean(const std::vector<float>& v);
int sign(float val);

std::vector<float> extractFeatures(std::vector<float> single_cycle, int tile_num = 15);

#endif  // WAVETABLE_FEATURES_H

