#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "DelayLine.h"
#include "Model.h"

class Processor : public juce::AudioProcessor,
                  public juce::AudioProcessorValueTreeState::Listener {
public:
    using State = juce::AudioProcessorValueTreeState;
    using ParameterLayout = juce::AudioProcessorValueTreeState::ParameterLayout;

    Processor();
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    bool hasEditor() const override;
    juce::AudioProcessorEditor* createEditor() override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    State& getState();

protected:
    bool isBusesLayoutSupported(const BusesLayout& layout) const override;

private:
    using Oscillator = juce::dsp::Oscillator<float>;

    static BusesProperties getBusesProperties();
    static ParameterLayout getParameterLayout();
    static double calculateDelayInSamples(float coarse, double sampleRate);

    static constexpr int maxNumChannels { 1 };
    static constexpr double delayElement_ms { 0.008f };
    static constexpr float lfoMax { 4.0f };
    static constexpr float lfoMin { 1.0f };

    State state;

    float mix { 0.0f };
    float regen { 0.0f };
    bool sf { false };
    float coarse { 1.0f };
    float coarseMapped { 0.0f };
    float fine { 1.0f };
    float fineMapped { 1.0f };
    float rate { 1.0f };
    float depth { 0.0f };

    DelayLine delayLine;
    Model model;
    Oscillator oscillator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Processor)
};
