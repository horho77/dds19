#pragma once

#include "Processor.h"

#include <juce_audio_processors/juce_audio_processors.h>

class Editor : public juce::AudioProcessorEditor {
public:
    explicit Editor(Processor& p);
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    using Slider = juce::Slider;
    using Button = juce::TextButton;
    using Label = juce::Label;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    Slider mix;
    Slider regen;
    Slider coarse;
    Slider fine;
    Slider rate;
    Slider depth;
    Button sf {"S/F"};

    Label mixLabel { {}, "MIX" };
    Label regenLabel { {}, "REGEN" };
    Label coarseLabel { {}, "COARSE" };
    Label fineLabel { {}, "FINE" };
    Label rateLabel { {}, "RATE" };
    Label depthLabel { {}, "DEPTH" };

    Label mixLeftLabel { {}, "D" };
    Label regenLeftLabel { {}, "0" };
    Label coarseLeftLabel { {}, "MAX" };
    Label fineLeftLabel { {}, "MAX" };
    Label rateLeftLabel { {}, "SLOW" };
    Label depthLeftLabel { {}, "0" };

    Label mixRightLabel { {}, "E" };
    Label regenRightLabel { {}, "MAX" };
    Label coarseRightLabel { {}, "MIN" };
    Label fineRightLabel { {}, "MIN" };
    Label rateRightLabel { {}, "FAST" };
    Label depthRightLabel { {}, "MAX" };

    SliderAttachment mixAttachment;
    SliderAttachment regenAttachment;
    SliderAttachment coarseAttachment;
    SliderAttachment fineAttachment;
    SliderAttachment rateAttachment;
    SliderAttachment depthAttachment;
    ButtonAttachment sfAttachment;
};
