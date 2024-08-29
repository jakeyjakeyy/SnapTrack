/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <thread>
#include <atomic>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

//==============================================================================
/**
*/
class DAWVSCAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    DAWVSCAudioProcessor();
    ~DAWVSCAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::String executeCommand(const std::string& command);

    void setProjectPath(const juce::String& path);
    juce::String getProjectPath();

    void checkForGit(const juce::String& path);

    juce::String getOS();
    juce::String getGitVersion();

    void checkGitStatus();

    void reloadWorkingTree();

    juce::StringArray getCommitHistory();

    using CommitHistoryChangedCallback = std::function<void()>;
    void setCommitHistoryChangedCallback(CommitHistoryChangedCallback callback);
    juce::String getCurrentBranch();
    juce::StringArray getBranches();

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DAWVSCAudioProcessor)
    std::unique_ptr<juce::File> projectPath;
    juce::String os;
    juce::String gitVersion;
    bool createdEditor = false; // We need to create the editor only once to prevent the bug where the terminal shows on relaunch
    CommitHistoryChangedCallback commitHistoryChangedCallback;
};
