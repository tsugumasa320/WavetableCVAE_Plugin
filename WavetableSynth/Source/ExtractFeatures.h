/*
  =============================================================================

    ExtractFeatures.
    Author:  湯谷承

  =============================================================================
*

#pragma onc

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
