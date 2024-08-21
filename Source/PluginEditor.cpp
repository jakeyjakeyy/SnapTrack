#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

DAWVSCAudioProcessorEditor::DAWVSCAudioProcessorEditor(DAWVSCAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), commitListBoxModel(commitHistory)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);

    // Initialize buttons
    addAndMakeVisible(browseButton);
    browseButton.setButtonText("Browse...");
    browseButton.onClick = [this] { browseButtonClicked(); };
    browseButton.setBounds(10, 10, getWidth() - 20, 20);

    addAndMakeVisible(checkoutButton);
    addAndMakeVisible(branchButton);
    addAndMakeVisible(mergeButton);
    addAndMakeVisible(goForwardButton);
    checkoutButton.setButtonText("Checkout");
    branchButton.setButtonText("Branch");
    mergeButton.setButtonText("Merge");
    goForwardButton.setButtonText("Redo");
    checkoutButton.onClick = [this] { checkoutButtonClicked(); };
    goForwardButton.onClick = [this] { goForwardButtonClicked(); };
    checkoutButton.setBounds(10, 40, 100, 20);
    branchButton.setBounds(110, 40, 50, 20);
    mergeButton.setBounds(160, 40, 50, 20);
    goForwardButton.setBounds(220, 40, 100, 20);

    // Initialize ListBox
    addAndMakeVisible(commitListBox);
    commitListBox.setModel(&commitListBoxModel);
    commitListBox.setBounds(10, 70, getWidth() - 20, getHeight() - 80);
    commitHistory = audioProcessor.getCommitHistory();
    commitListBox.updateContent();
    audioProcessor.setCommitHistoryChangedCallback([this] { refreshCommitListBox(); });


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
    }

    resString = "Editor created\n";
    result.append(resString, resString.length());
    debugText.setText(result, juce::dontSendNotification);
}

DAWVSCAudioProcessorEditor::~DAWVSCAudioProcessorEditor()
{
}

//==============================================================================

void DAWVSCAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f));
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void DAWVSCAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    browseButton.setBounds(10, 10, getWidth() - 20, 20);
    checkoutButton.setBounds(10, 40, 100, 20);
    branchButton.setBounds(110, 40, 50, 20);
    mergeButton.setBounds(160, 40, 50, 20);
    goForwardButton.setBounds(220, 40, 100, 20);
    commitListBox.setBounds(10, 70, getWidth() - 20, getHeight() - 80);
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
            }
        });
}

void DAWVSCAudioProcessorEditor::checkoutButtonClicked()
{
    commitHistory = audioProcessor.getCommitHistory();
    commitListBox.updateContent();
}

void DAWVSCAudioProcessorEditor::goForwardButtonClicked()
{
    juce::String res = "";
    res = audioProcessor.executeCommand("git status");
    if (res.contains("HEAD detached at"))
    {
        audioProcessor.executeCommand("git checkout master");
        audioProcessor.reloadWorkingTree();
    }
}

void DAWVSCAudioProcessorEditor::refreshCommitListBox()
{
    commitHistory = audioProcessor.getCommitHistory();
    commitListBox.updateContent();
}