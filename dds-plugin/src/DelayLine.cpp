#include "DelayLine.h"

#include <cmath>

void DelayLine::setDelayInSamples(double delayInSamples, float sampleRateRatio)
{
    delayMin = delayInSamples;
    delayMax = delayInSamples * 8.0;
    outputOffset = delayMin * sampleRateRatio - delayInSamples;
}

void DelayLine::in(float sample, float inputSampleRateRatio)
{
    inputIndex = ++inputIndex % dataBuf.size();
    dataBuf[inputIndex] = sample;
    sampleRateRatioBuf[inputIndex] = inputSampleRateRatio;
}

float DelayLine::out(float outputSampleRateRatio, float lfo, float depth)
{
    const auto inputSampleRateRatio = getInputSampleRateRatio();
    const auto lfoExp = std::pow(4, lfo) - 1.3525266;

    const double sampleRateRatio = static_cast<double>(inputSampleRateRatio) / outputSampleRateRatio;
    auto delta = sampleRateRatio + lfoExp * depth * 0.7;
    auto delayDiff = delayMax - delayMin;

    outputOffset += 1.0 - delta;
    if (outputOffset < 0.0)
        outputOffset = 0.0;
    else if (outputOffset > delayDiff)
        outputOffset = delayDiff;

    auto index = getOutputIndex(outputOffset);
    return readDataAt(index);
}

double DelayLine::getOutputIndex(double offset)
{
    const auto maxIndex = static_cast<double>(dataBuf.size());
    auto index = static_cast<double>(inputIndex) + maxIndex - (delayMin + offset);
    if (index >= maxIndex)
        index -= maxIndex;

    return index;
}

float DelayLine::readDataAt(double index)
{
    const auto maxIndex = dataBuf.size();
    const auto preIndex = static_cast<size_t>(index);
    const auto postIndex = static_cast<size_t>(index + 1.0f);

    const auto preSample = dataBuf[preIndex % maxIndex];
    const auto postSample = dataBuf[postIndex % maxIndex];

    const auto remainder = index - static_cast<double>(static_cast<size_t>(index));
    const auto deltaSample = preSample - postSample;
    const auto sample = preSample - deltaSample * static_cast<float>(remainder);

    return sample;
}

float DelayLine::getInputSampleRateRatio()
{
    const auto delay = static_cast<size_t>(delayMin + outputOffset);
    const auto maxIndex = dataBuf.size();

    size_t index;
    if (inputIndex >= delay)
        index = inputIndex - delay;
    else
        index = maxIndex - (delay - inputIndex);

    return sampleRateRatioBuf[index];
}
