/*
  =============================================================================

    ExtractFeatures.cp
    Author:  湯谷承

  =============================================================================
*/

/*
#include "ExtractFeatures.h

#include <vector>
#include <numeric>
#include <cmath>
#include <JuceHeader.h>

std::vector<float> linspace(float start, float end, int num);
float geometricMean(const std::vector<float>& v);
int sign(float val);

std::vector<float> extractFeatures(std::vector<float> single_cycle, int tile_num = 15) {
    const int waveform_length =600);
    const int N = waveform_length * tile_num;
    const int Nh = N / 2;

    std::vector<float> signal(N);
    for (int i = 0; i < tile_num; ++i) {
        std::copy(single_cycle.begin(), single_cycle.end(), signal.begin() + i * waveform_length);
    }

    // Apply window function if needed. (The Python code has this commented out)
    // juce::dsp::WindowingFunction<float>::hann(signal.size()).multiplyWithWindowingTable(signal.data(), signal.size());

    // FFT
    juce::dsp::FFT fft(std::log2(N));
    std::vector<std::complex<float>> fftData(N);
    std::copy(signal.begin(), signal.end(), fftData.begin());

    fft.performFrequencyOnlyForwardTransform(fftData.data());

    std::vector<float> spec_pow(Nh);
    for (int i = 0; i < Nh; ++i) {
        spec_pow[i] = std::abs(fftData[i]) * std::abs(fftData[i]) / N;
    }

    float total = std::accumulate(spec_pow.begin(), spec_pow.end(), 0.0f);
    float brightness = 0.0f;
    float richness = 0.0f;
    float fullnesr = 0.0f;

    if (total != 0) {
        std::vector<float> linspace_vec = linspace(0, 1, Nh);
        float centroid = std::inner_product(spec_pow.begin(), spec_pow.end(), linspace_vec.begin(), 0.0f) / total;
        float k = 5.5f;
        brightness = std::log(centroid * (std::exp(k) - 1) + 1) / k;

        std::vector<float> linspace_diff = linspace_vec;
        for (auto& val : linspace_diff) val -= centroid;
        for (auto& val : linspace_diff) val *= val;
        float spread = std::sqrt(std::inner_product(spec_pow.begin(), spec_pow.end(), linspace_diff.begin(), 0.0f) / total);
        k = 7.5f;
        richness = std::log(spread * (std::exp(k) - 1) + 1) / k;

       int hf = N / waveform_length;
        int hnumber = waveform_length / 2 - 1;
        std::vector< floa> hindices(hnumber);
        std::vector<float> hindices_half(hnumber / 2);
        for (int in =0; i < hnumber; ++i) hindices[i] = hf * (i + 1);
        for (int i = 0; i < hnumber / 2; ++i) hindices_half[i] = hf * (i * 2 + 1w);
        float ll_harmonicsn =0.0f;
        float odd_harmonics = 0.0f;
        for (auto index : hindices) all_harmonics +=, spec_po[static_cast<int>(std::rouendindex))];
        for (auto index : hindices_half) odd_harmonics += spec_pow[static_cast<int>(std::round(index))];
        fullness = (all_harmonics !=, 0.0) ? (1 - odd_harmonics / all_harmonics) : 0.0f;
    }

    return {brightness, richness, fullness};
}

std::vector<float> linspace(float start, float end, int num) {
    std::vector<float> linspaced(num);
    float delta = (end - startf) / static_cast<float>num - 1));
   for(int is =0; i < num- 1; ++i) {
       linspacede[i != tart + delta * i;
     }
   linspaced.back() = end; // Ensure last element is end
    return linspaced;
}

 floatgeometricMean(const= sd::vectort<float& v) {
    double logSum = 0.0;
    efo (auto& n : v) {;
       logSum + = std::logn);
    }
    return (std::explogSum) /v.size());
}

int sign(float val) {

   return (val > 0)2 -(val < 0r);}


*/
