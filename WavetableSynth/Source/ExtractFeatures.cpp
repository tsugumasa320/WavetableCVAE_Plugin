#include <vector>
#include <numeric>
#include <cmath>
#include <JuceHeader.h>
#include "ExtractFeatures.h"

std::vector<float> linspace(float start, float end, int num);
float geometricMean(const std::vector<float>& v);
int sign(float val);

std::vector<float> extractFeatures(std::vector<float> single_cycle, int tile_num) {
    const int waveform_length = 600;
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
    
    std::vector<float> fftData(N * 2, 0.0f);
    for (int i = 0; i < N; ++i) {
        fftData[i * 2] = signal[i]; // real part
        fftData[i * 2 + 1] = 0.0f;  // imaginary part is set to 0
    }
    
    std::copy(signal.begin(), signal.end(), fftData.begin());

    fft.performFrequencyOnlyForwardTransform(fftData.data());

    std::vector<float> spec_pow(Nh);
    for (int i = 0; i < Nh; ++i) {
        // Calculate magnitude from real and imaginary parts, then calculate power
        float magnitude = std::sqrt(std::pow(fftData[i * 2], 2) + std::pow(fftData[i * 2 + 1], 2));
        spec_pow[i] = std::pow(magnitude, 2) / N;
    }

    float total = std::accumulate(spec_pow.begin(), spec_pow.end(), 0.0f);
    float brightness = 0.0f;
    float richness = 0.0f;
    float noiseness_fl = 0.0f;
    float fullness = 0.0f;
    float noiseness_zcr = 0.0f;

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

        float gmean = geometricMean(spec_pow);
        float amean = std::accumulate(spec_pow.begin(), spec_pow.end(), 0.0f) / static_cast<float>(spec_pow.size());
        float flatness = gmean / amean;
        k = 5.5f;
        noiseness_fl = std::log(flatness * (std::exp(k) - 1) + 1) / k;

        int zc = 0;
        for (int i = 0; i < waveform_length - 1; ++i) {
            zc += (sign(single_cycle[i]) != sign(single_cycle[i+1])) ? 1 : 0;
        }
        float zero_crossing_rate = static_cast<float>(zc) / waveform_length;
        k = 5.5f;
        noiseness_zcr = std::log(zero_crossing_rate * (std::exp(k) - 1) + 1) / k;

        int hf = N / waveform_length;
        int hnumber = waveform_length / 2 - 1;
        std::vector<float> hindices(hnumber);
        std::vector<float> hindices_half(hnumber / 2);
        for (int i = 0; i < hnumber; ++i) hindices[i] = hf * (i + 1);
        for (int i = 0; i < hnumber / 2; ++i) hindices_half[i] = hf * (i * 2 + 1);
        float all_harmonics = 0.0f;
        float odd_harmonics = 0.0f;
        for (auto index : hindices) all_harmonics += spec_pow[static_cast<int>(std::round(index))];
        for (auto index : hindices_half) odd_harmonics += spec_pow[static_cast<int>(std::round(index))];
        fullness = (all_harmonics != 0.0f) ? (1 - odd_harmonics / all_harmonics) : 0.0f;
    }

    return {brightness, richness, noiseness_fl, fullness, noiseness_zcr};
}

std::vector<float> linspace(float start, float end, int num) {
    std::vector<float> linspaced(num);
    float delta = (end - start) / static_cast<float>(num - 1);
    for(int i = 0; i < num-1; ++i) {
        linspaced[i] = start + delta * i;
    }
    linspaced.back() = end; // Ensure last element is end
    return linspaced;
}

float geometricMean(const std::vector<float>& v) {
    double logSum = 0.0;
    for (auto& n : v) {
        logSum += std::log(n);
    }
    return std::exp(logSum / v.size());
}

int sign(float val) {
    return (val > 0) - (val < 0);
}
