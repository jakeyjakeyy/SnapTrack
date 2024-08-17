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
    setSize (400, 300);

    addAndMakeVisible(browseButton);
    browseButton.setButtonText("Browse...");
    browseButton.onClick = [this] { browseButtonClicked(); };
    browseButton.setBounds(10, 40, getWidth() - 20, 20);

    addAndMakeVisible(debugText);
    debugText.setBounds(10, 70, getWidth() - 20, getHeight() - 80);

    projectPath = audioProcessor.getProjectPath();
    DBG("Project path: " << projectPath);
    debugText.setText(projectPath, juce::dontSendNotification);
    if (projectPath.isNotEmpty())
	{
        checkForGit(projectPath);
	}
    DBG("Editor created");
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
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
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
                debugText.setText(audioProcessor.getProjectPath(), juce::dontSendNotification);
			}
		});
}

void DAWVSCAudioProcessorEditor::checkForGit(const juce::String& path)
{
    juce::File projectDir(path);
    juce::Array<juce::File> gitFolders;
    projectDir.findChildFiles(gitFolders, juce::File::findDirectories, false, ".git");

    if (gitFolders.isEmpty())
    {
        debugText.setText("No Git repository found at: " + path, juce::dontSendNotification);
    }
    else
    {
        debugText.setText("Git repository found at: " + path, juce::dontSendNotification);
    }
}