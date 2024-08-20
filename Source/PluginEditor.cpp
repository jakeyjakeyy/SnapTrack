/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DAWVSCAudioProcessorEditor::DAWVSCAudioProcessorEditor (DAWVSCAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 150);

    // Initialize buttons
    addAndMakeVisible(browseButton);
    browseButton.setButtonText("Browse...");
    browseButton.onClick = [this] { browseButtonClicked(); };
    browseButton.setBounds(10, 10, getWidth() - 20, 20);

    addAndMakeVisible(goBackButton);
    addAndMakeVisible(branchButton);
    addAndMakeVisible(mergeButton);
    addAndMakeVisible(goForwardButton);
    goBackButton.setButtonText("Go back");
    branchButton.setButtonText("Branch");
    mergeButton.setButtonText("Merge");
    goForwardButton.setButtonText("Redo");
    goBackButton.onClick = [this] { goBackButtonClicked(); };
    goForwardButton.onClick = [this] { goForwardButtonClicked(); };
    goBackButton.setBounds(10, 40, 100, 20);
    branchButton.setBounds(110, 40, 50, 20);
    mergeButton.setBounds(160, 40, 50, 20);
    goForwardButton.setBounds(220, 40, 100, 20);
    
    addAndMakeVisible(debugText);
    debugText.setBounds(10, 65, getWidth() - 20, getHeight() - 65);
    // Fetch OS
    resString = "OS: " + audioProcessor.getOS() + "\n";
    result.append(resString, resString.length());

    // Check git installation
    juce::String gitVersion = audioProcessor.getGitVersion();
    if (gitVersion.isEmpty())
	{
		resString = "Git not installed\n";
		result.append(resString, resString.length());
	}
	else
	{
		resString = "Git version: " + gitVersion + "\n";
		result.append(resString, resString.length());
	}

    // Get project path
    projectPath = audioProcessor.getProjectPath();
    DBG("Project path: " + projectPath);
    resString = "Project path: " + projectPath + "\n";
    result.append(resString, resString.length());

    // Check for git repository in project path
    if (projectPath.isNotEmpty())
	{
        audioProcessor.checkForGit(projectPath);
        //debugText.setText(result, juce::dontSendNotification);
	}

    resString = "Editor created\n";
    result.append(resString, resString.length());
    debugText.setText(result, juce::dontSendNotification);
}

DAWVSCAudioProcessorEditor::~DAWVSCAudioProcessorEditor()
{
}

//==============================================================================
void DAWVSCAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void DAWVSCAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    browseButton.setBounds(10, 40, getWidth() - 20, 20);

    debugText.setBounds(10, 70, getWidth() - 20, getHeight() - 80);

}

void DAWVSCAudioProcessorEditor::browseButtonClicked()
{
    chooser = std::make_unique<juce::FileChooser>("Select project directory", juce::File::getSpecialLocation(juce::File::userHomeDirectory), "*");

    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
		[this](const juce::FileChooser& fc)
		{
			if (fc.getResult().exists())
			{
                audioProcessor.setProjectPath(fc.getResult().getFullPathName());
                audioProcessor.checkForGit(audioProcessor.getProjectPath());
                //debugText.setText(result, juce::dontSendNotification);
			}
		});
}

void DAWVSCAudioProcessorEditor::goBackButtonClicked()
{
    juce::String res = "";
	res = audioProcessor.executeCommand("git log --pretty=oneline");
    juce::String hash = res.fromFirstOccurrenceOf("\n", false, true);
    hash = hash.upToFirstOccurrenceOf(" ", false, true);
    debugText.setText(hash, juce::dontSendNotification);
    res = "git checkout " + hash;
    audioProcessor.executeCommand(res.toStdString());
}

void DAWVSCAudioProcessorEditor::goForwardButtonClicked()
{
    juce::String res = "";
    res = audioProcessor.executeCommand("git status");
    if (res.contains("HEAD detached at"))
    {
        //juce::String hash = res.fromFirstOccurrenceOf("HEAD detached at ", false, true);
        //hash = hash.upToFirstOccurrenceOf("\n", false, true);
        audioProcessor.executeCommand("git checkout master");
    }
}