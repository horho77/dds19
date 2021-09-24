#pragma once

#include <array>

class DelayLine {
public:
    void setDelayInSamples(double delayInSamples, float sampleRateRatio);
    void in(float sample, float inputSampleRateRatio);
    float out(float outputSampleRateRatio, float lfo, float depth);

private:
    double getOutputIndex(double offset);
    float readDataAt(double index);
    float getInputSampleRateRatio();

    static constexpr size_t maxDelay_ms { 8192 };
    static constexpr size_t maxSampleRate_kHz { 192 };

    std::array<float, maxDelay_ms * maxSampleRate_kHz> dataBuf {};
    std::array<float, maxDelay_ms * maxSampleRate_kHz> sampleRateRatioBuf {};

    size_t inputIndex { 0 };
    double outputOffset { 0.0 };
    double delayMin { 0.0 };
    double delayMax { 0.0 };
};
