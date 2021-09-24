#include "Editor.h"

Editor::Editor(Processor& p)
    : juce::AudioProcessorEditor(p)
    , mixAttachment(p.getState(), "mix", mix)
    , regenAttachment(p.getState(), "regen", regen)
    , coarseAttachment(p.getState(), "coarse", coarse)
    , fineAttachment(p.getState(), "fine", fine)
    , rateAttachment(p.getState(), "rate", rate)
    , depthAttachment(p.getState(), "depth", depth)
    , sfAttachment(p.getState(), "sf", sf)
{
    juce::ignoreUnused(mixAttachment);
    juce::ignoreUnused(regenAttachment);
    juce::ignoreUnused(coarseAttachment);
    juce::ignoreUnused(fineAttachment);
    juce::ignoreUnused(rateAttachment);
    juce::ignoreUnused(depthAttachment);
    juce::ignoreUnused(sfAttachment);

    setSize(700, 130);

    addAndMakeVisible(mix);
    mix.setSliderStyle(Slider::Rotary);
    mix.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(regen);
    regen.setSliderStyle(Slider::Rotary);
    regen.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(sf);
    sf.setClickingTogglesState(true);
    sf.setColour(Button::ColourIds::buttonOnColourId, juce::Colours::crimson);

    addAndMakeVisible(coarse);
    coarse.setSliderStyle(Slider::Rotary);
    coarse.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(fine);
    fine.setSliderStyle(Slider::Rotary);
    fine.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(rate);
    rate.setSliderStyle(Slider::Rotary);
    rate.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    addAndMakeVisible(depth);
    depth.setSliderStyle(Slider::Rotary);
    depth.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);

    auto labelFont = juce::Font(15.0f);
    mixLabel.setFont(labelFont);
    regenLabel.setFont(labelFont);
    coarseLabel.setFont(labelFont);
    fineLabel.setFont(labelFont);
    rateLabel.setFont(labelFont);
    depthLabel.setFont(labelFont);
    mixLabel.setJustificationType(juce::Justification::centredTop);
    regenLabel.setJustificationType(juce::Justification::centredTop);
    coarseLabel.setJustificationType(juce::Justification::centredTop);
    fineLabel.setJustificationType(juce::Justification::centredTop);
    rateLabel.setJustificationType(juce::Justification::centredTop);
    depthLabel.setJustificationType(juce::Justification::centredTop);
    addAndMakeVisible(mixLabel);
    addAndMakeVisible(regenLabel);
    addAndMakeVisible(coarseLabel);
    addAndMakeVisible(fineLabel);
    addAndMakeVisible(rateLabel);
    addAndMakeVisible(depthLabel);

    auto labelDetailsFont = juce::Font(13.0f);
    mixLeftLabel.setFont(labelDetailsFont);
    regenLeftLabel.setFont(labelDetailsFont);
    coarseLeftLabel.setFont(labelDetailsFont);
    fineLeftLabel.setFont(labelDetailsFont);
    rateLeftLabel.setFont(labelDetailsFont);
    depthLeftLabel.setFont(labelDetailsFont);
    mixLeftLabel.setJustificationType(juce::Justification::bottomRight);
    regenLeftLabel.setJustificationType(juce::Justification::bottomRight);
    coarseLeftLabel.setJustificationType(juce::Justification::bottomRight);
    fineLeftLabel.setJustificationType(juce::Justification::bottomRight);
    rateLeftLabel.setJustificationType(juce::Justification::bottomRight);
    depthLeftLabel.setJustificationType(juce::Justification::bottomRight);
    addAndMakeVisible(mixLeftLabel);
    addAndMakeVisible(regenLeftLabel);
    addAndMakeVisible(coarseLeftLabel);
    addAndMakeVisible(fineLeftLabel);
    addAndMakeVisible(rateLeftLabel);
    addAndMakeVisible(depthLeftLabel);

    mixRightLabel.setFont(labelDetailsFont);
    regenRightLabel.setFont(labelDetailsFont);
    coarseRightLabel.setFont(labelDetailsFont);
    fineRightLabel.setFont(labelDetailsFont);
    rateRightLabel.setFont(labelDetailsFont);
    depthRightLabel.setFont(labelDetailsFont);
    mixRightLabel.setJustificationType(juce::Justification::bottomLeft);
    regenRightLabel.setJustificationType(juce::Justification::bottomLeft);
    coarseRightLabel.setJustificationType(juce::Justification::bottomLeft);
    fineRightLabel.setJustificationType(juce::Justification::bottomLeft);
    rateRightLabel.setJustificationType(juce::Justification::bottomLeft);
    depthRightLabel.setJustificationType(juce::Justification::bottomLeft);
    addAndMakeVisible(mixRightLabel);
    addAndMakeVisible(regenRightLabel);
    addAndMakeVisible(coarseRightLabel);
    addAndMakeVisible(fineRightLabel);
    addAndMakeVisible(rateRightLabel);
    addAndMakeVisible(depthRightLabel);
}

void Editor::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black);
    g.fillAll();
}

void Editor::resized()
{
    auto area = getLocalBounds().reduced(8);
    auto itemWidth = area.getWidth() / 7;
    auto spacing = itemWidth / 6;
    auto labelHeight = 15;
    auto labelDetailsHeight = 13;
    auto detailsOffset = static_cast<int>(itemWidth * 0.6);

    auto mixArea = area.removeFromLeft(itemWidth);
    auto regenArea = area.removeFromLeft(itemWidth);
    auto sfArea = area.removeFromLeft(itemWidth - spacing);
    auto coarseArea = area.removeFromLeft(itemWidth);
    auto fineArea = area.removeFromLeft(itemWidth);
    area.removeFromLeft(spacing);
    auto rateArea = area.removeFromLeft(itemWidth);
    auto depthArea = area.removeFromLeft(itemWidth);

    mixLabel.setBounds(mixArea.removeFromTop(labelHeight));
    regenLabel.setBounds(regenArea.removeFromTop(labelHeight));
    sfArea.removeFromTop(labelHeight);
    coarseLabel.setBounds(coarseArea.removeFromTop(labelHeight));
    fineLabel.setBounds(fineArea.removeFromTop(labelHeight));
    rateLabel.setBounds(rateArea.removeFromTop(labelHeight));
    depthLabel.setBounds(depthArea.removeFromTop(labelHeight));

    auto mixDetailsArea = mixArea.removeFromBottom(labelDetailsHeight);
    auto regenDetailsArea = regenArea.removeFromBottom(labelDetailsHeight);
    sfArea.removeFromBottom(labelDetailsHeight);
    auto coarseDetailsArea = coarseArea.removeFromBottom(labelDetailsHeight);
    auto fineDetailsArea = fineArea.removeFromBottom(labelDetailsHeight);
    auto rateDetailsArea = rateArea.removeFromBottom(labelDetailsHeight);
    auto depthDetailsArea = depthArea.removeFromBottom(labelDetailsHeight);

    mix.setBounds(mixArea);
    regen.setBounds(regenArea);
    sf.setBounds(sfArea.reduced(spacing, spacing * 2));
    coarse.setBounds(coarseArea);
    fine.setBounds(fineArea);
    rate.setBounds(rateArea);
    depth.setBounds(depthArea);

    auto mixLeftArea = mixDetailsArea;
    auto regenLeftArea = regenDetailsArea;
    auto coarseLeftArea = coarseDetailsArea;
    auto fineLeftArea = fineDetailsArea;
    auto rateLeftArea = rateDetailsArea;
    auto depthLeftArea = depthDetailsArea;
    mixLeftArea.removeFromRight(detailsOffset);
    regenLeftArea.removeFromRight(detailsOffset);
    coarseLeftArea.removeFromRight(detailsOffset);
    fineLeftArea.removeFromRight(detailsOffset);
    rateLeftArea.removeFromRight(detailsOffset);
    depthLeftArea.removeFromRight(detailsOffset);

    auto mixRightArea = mixDetailsArea;
    auto regenRightArea = regenDetailsArea;
    auto coarseRightArea = coarseDetailsArea;
    auto fineRightArea = fineDetailsArea;
    auto rateRightArea = rateDetailsArea;
    auto depthRightArea = depthDetailsArea;
    mixRightArea.removeFromLeft(detailsOffset);
    regenRightArea.removeFromLeft(detailsOffset);
    coarseRightArea.removeFromLeft(detailsOffset);
    fineRightArea.removeFromLeft(detailsOffset);
    rateRightArea.removeFromLeft(detailsOffset);
    depthRightArea.removeFromLeft(detailsOffset);

    mixLeftLabel.setBounds(mixLeftArea);
    regenLeftLabel.setBounds(regenLeftArea);
    coarseLeftLabel.setBounds(coarseLeftArea);
    fineLeftLabel.setBounds(fineLeftArea);
    rateLeftLabel.setBounds(rateLeftArea);
    depthLeftLabel.setBounds(depthLeftArea);

    mixRightLabel.setBounds(mixRightArea);
    regenRightLabel.setBounds(regenRightArea);
    coarseRightLabel.setBounds(coarseRightArea);
    fineRightLabel.setBounds(fineRightArea);
    rateRightLabel.setBounds(rateRightArea);
    depthRightLabel.setBounds(depthRightArea);
}
