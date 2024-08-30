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
                    g.fillAll(juce::Colour(212, 163, 115));

                g.setColour(juce::Colour(6, 6, 5));
                g.setFont(height * 0.5f);
                g.drawText(commitHistory[rowNumber], 5, 0, width, height, juce::Justification::centredLeft, true);
            }

        private:
            juce::StringArray& commitHistory;
    };

    juce::ListBox branchListBox;
    juce::StringArray branchList;
    class BranchListBoxModel : public juce::ListBoxModel
    {
    public:
        using ItemClickedCallback = std::function<void(int)>;

        BranchListBoxModel(juce::StringArray& branches, ItemClickedCallback callback)
            : branchList(branches), itemClickedCallback(callback) {}

        int getNumRows() override
        {
            return branchList.size();
        }

        void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
        {
            if (rowIsSelected)
                g.fillAll(juce::Colour(212, 163, 115));

            g.setColour(juce::Colour(6, 6, 5));
            g.setFont(height * 0.5f);
            g.drawText(branchList[rowNumber], 5, 0, width, height, juce::Justification::centredLeft, true);
        }

        void listBoxItemClicked(int row, const juce::MouseEvent&) override
        {
            if (itemClickedCallback)
                itemClickedCallback(row);
        }

    private:
        juce::StringArray& branchList;
        ItemClickedCallback itemClickedCallback;
    };

    CommitListBoxModel commitListBoxModel;
    BranchListBoxModel branchListBoxModel;

    DAWVSCAudioProcessor& audioProcessor;

    // Init Browse button (when no ProjectPath is set)
    juce::TextButton browseButton;
    // Branch Controls
    juce::TextButton branchButton;
    juce::TextButton deleteBranchButton;
    juce::TextButton mergeButton;
    // Commit Controls
    juce::TextButton commitButton;
    juce::TextButton checkoutButton;
    juce::TextButton goForwardButton;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::String projectPath;
    juce::Label debugText;

    void browseButtonClicked();
    void checkoutButtonClicked();
    void goForwardButtonClicked();
    void branchButtonClicked();
    void deleteBranchButtonClicked();
    void mergeButtonClicked();
    void commitButtonClicked();

    void executeAndRefresh(juce::String command);

    void refreshCommitListBox();
    void refreshBranchListBox();

    std::unique_ptr<juce::AlertWindow> alertWindow;

    void onBranchListItemClicked(int row);

    juce::Colour textColor;
    juce::Colour backgroundColor;
    juce::Colour secondaryBackgroundColor;
    juce::Colour primaryColor;
    juce::Colour secondaryColor;
    juce::Colour accentColor;

    juce::LookAndFeel_V4 customLookAndFeel;

    bool gitInstalled;
    juce::String gitVersion;
    juce::String gitNotInstalledMessage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DAWVSCAudioProcessorEditor)
};
