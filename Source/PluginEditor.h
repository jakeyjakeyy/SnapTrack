/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class DAWVSCAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DAWVSCAudioProcessorEditor (DAWVSCAudioProcessor&);
    ~DAWVSCAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DAWVSCAudioProcessor& audioProcessor;

    juce::Label debugText;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::TextButton browseButton;

    juce::TextButton goBackButton;

    juce::TextButton branchButton;

    juce::TextButton mergeButton;

    juce::TextButton goForwardButton;
    
    juce::String projectPath;

    juce::String result;

    juce::String resString;

    void browseButtonClicked();
    
    void goBackButtonClicked();

    void goForwardButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DAWVSCAudioProcessorEditor)
};
