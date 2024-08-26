#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class DAWVSCAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DAWVSCAudioProcessorEditor(DAWVSCAudioProcessor&);
    ~DAWVSCAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::ListBox commitListBox;
    juce::StringArray commitHistory;
    juce::StringArray commitHashes;
    class CommitListBoxModel : public juce::ListBoxModel
    {
    public:
        CommitListBoxModel(juce::StringArray& commits) : commitHistory(commits) {}

        int getNumRows() override
        {
            return commitHistory.size();
        }

        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
        {
            if (rowIsSelected)
                g.fillAll(juce::Colours::lightblue);

            g.setColour(juce::Colours::black);
            g.setFont(height * 0.7f);
            g.drawText(commitHistory[rowNumber], 5, 0, width, height, juce::Justification::centredLeft, true);
        }

    private:
        juce::StringArray& commitHistory;
    };

    CommitListBoxModel commitListBoxModel;

    DAWVSCAudioProcessor& audioProcessor;
    juce::TextButton browseButton;
    juce::TextButton checkoutButton;
    juce::TextButton branchButton;
    juce::TextButton deleteBranchButton;
    juce::TextButton mergeButton;
    juce::TextButton goForwardButton;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::String projectPath;
    juce::String resString;
    juce::String result;
    juce::Label debugText;

    void browseButtonClicked();
    void checkoutButtonClicked();
    void goForwardButtonClicked();
    void branchButtonClicked();
    void deleteBranchButtonClicked();
    void mergeButtonClicked();

    void executeAndRefresh(juce::String command);

    void refreshCommitListBox();

    std::unique_ptr<juce::AlertWindow> alertWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DAWVSCAudioProcessorEditor)
};
