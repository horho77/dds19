#include "Processor.h"
#include "Editor.h"

#include <fmt/core.h>

Processor::Processor()
    : AudioProcessor(getBusesProperties())
    , state(*this, nullptr, "state", getParameterLayout())
{
    state.addParameterListener("mix", this);
    state.addParameterListener("regen", this);
    state.addParameterListener("sf", this);
    state.addParameterListener("coarse", this);
    state.addParameterListener("fine", this);
    state.addParameterListener("rate", this);
    state.addParameterListener("depth", this);

    mix = state.getRawParameterValue("mix")->load();
    regen = state.getRawParameterValue("regen")->load();
    sf = static_cast<bool>(state.getRawParameterValue("sf")->load());
    coarse = state.getRawParameterValue("coarse")->load();
    coarseMapped = juce::jmap(coarse, 0.0f, 1.0f, 1.0f, 0.0f);
    fine = state.getRawParameterValue("fine")->load();
    fineMapped = juce::jmap(fine, 0.0f, 1.0f, 4.0f, 1.0f);
    rate = state.getRawParameterValue("rate")->load();
    depth = state.getRawParameterValue("depth")->load();
}

const juce::String Processor::getName() const
{
    return JucePlugin_Name;
}

bool Processor::acceptsMidi() const
{
    return false;
}

bool Processor::producesMidi() const
{
    return false;
}

bool Processor::isMidiEffect() const
{
    return false;
}

double Processor::getTailLengthSeconds() const
{
    return 0.f;
}

int Processor::getNumPrograms()
{
    return 1;
}

int Processor::getCurrentProgram()
{
    return 0;
}

void Processor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String Processor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return juce::String();
}

void Processor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void Processor::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
{
    juce::dsp::ProcessSpec spec {
        sampleRate,
        static_cast<juce::uint32>(maximumExpectedSamplesPerBlock),
        static_cast<juce::uint32>(getTotalNumInputChannels())
    };

    fmt::print("prepareToPlay()\n");
    fmt::print("Sample rate: {}\n", spec.sampleRate);
    fmt::print("Maximum expected samples per block: {}\n", spec.maximumBlockSize);
    fmt::print("Num channels: {}\n", spec.numChannels);

    oscillator.reset();
    oscillator.prepare(spec);
    // triangle wave (-1.0 to 1.0)
    oscillator.initialise([](float x) { return (2 / juce::float_Pi) * std::asin(std::sin(x)); });
    oscillator.setFrequency(rate);

    auto delayInSamples = calculateDelayInSamples(coarseMapped, getSampleRate());
    delayLine.setDelayInSamples(delayInSamples, sf ? fineMapped * 2.0f : fineMapped);
}

void Processor::releaseResources()
{
    fmt::print("releaseResources()\n");
}

void Processor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto numChannels = getTotalNumInputChannels();
    for (auto channel = 0; channel < numChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);

        for (auto sampleIndex = 0; sampleIndex < buffer.getNumSamples(); sampleIndex++) {
            auto inputSample = channelData[sampleIndex];
            auto lfo = oscillator.processSample(0.0f);
            auto sampleRateRatio = sf ? fineMapped * 2.0f : fineMapped;
            auto outputSample = delayLine.out(sampleRateRatio, lfo, depth);
            auto modelOutputSample = model.process(outputSample, sf ? 1.0f : 0.0f, fine);
            delayLine.in(inputSample + modelOutputSample * regen, sampleRateRatio);
            channelData[sampleIndex] = inputSample * (1.0f - mix) + modelOutputSample * mix;
        }
    }
}

bool Processor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* Processor::createEditor()
{
    return new Editor(*this);
}

void Processor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xmlState = state.copyState().createXml())
        copyXmlToBinary(*xmlState, destData);
}

void Processor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
        state.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void Processor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "mix") {
        mix = newValue;
    } else if (parameterID == "regen") {
        regen = newValue;
    } else if (parameterID == "sf") {
        sf = static_cast<bool>(newValue);
    } else if (parameterID == "coarse") {
        coarse = newValue;
        coarseMapped = juce::jmap(coarse, 0.0f, 1.0f, 1.0f, 0.0f);
        auto delayInSamples = calculateDelayInSamples(coarseMapped, getSampleRate());
        delayLine.setDelayInSamples(delayInSamples, sf ? fineMapped * 2.0f : fineMapped);
    } else if (parameterID == "fine") {
        fine = newValue;
        fineMapped = juce::jmap(fine, 0.0f, 1.0f, 4.0f, 1.0f);
    } else if (parameterID == "rate") {
        rate = newValue;
        oscillator.setFrequency(rate);
    } else if (parameterID == "depth") {
        depth = newValue;
    }
}

Processor::State& Processor::getState()
{
    return state;
}

bool Processor::isBusesLayoutSupported(const juce::AudioProcessor::BusesLayout& layout) const
{
    if (layout.getMainOutputChannelSet().size() > maxNumChannels)
        return false;

    if (layout.getMainInputChannelSet() != layout.getMainOutputChannelSet())
        return false;

    return true;
}

juce::AudioProcessor::BusesProperties Processor::getBusesProperties()
{
    return BusesProperties()
        .withInput("Input", juce::AudioChannelSet::mono(), true)
        .withOutput("Output", juce::AudioChannelSet::mono(), true);
}

Processor::ParameterLayout Processor::getParameterLayout()
{
    return {
        std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("regen", "Regen", 0.0f, 1.0f, 0.0f),
        std::make_unique<juce::AudioParameterBool>("sf", "S/F", false),
        std::make_unique<juce::AudioParameterFloat>("coarse", "Coarse", 0.0f, 1.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("fine", "Fine", 0.0f, 1.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("rate", "Rate", 0.1f, 10.0f, 0.1f),
        std::make_unique<juce::AudioParameterFloat>("depth", "Depth", 0.0f, 1.0f, 0.0f),
    };
}

double Processor::calculateDelayInSamples(float coarse, double sampleRate)
{
    // coarse range: 0.0f - 1.0f
    // 0 - 0.99: 8ms
    // 1 - 1.99: 16ms
    // 2 - 2.99: 32ms
    // 3 - 3.99: 64ms
    // 4 - 4.99: 128ms
    // 5 - 5.99: 256ms
    // 6 - 6.99: 512ms
    // 7 - 7.99: 1024ms
    // 8 - 8.99: 2048ms
    // 9 - 9.99: 4096ms
    // 10 - 10.99: 8192ms
    auto coarseInt = static_cast<int>(coarse * 10.99f);

    auto delay { delayElement_ms * sampleRate };
    for (int i = 0; i < coarseInt; i++) {
        delay *= 2;
    }
    return delay - 1;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Processor();
}
