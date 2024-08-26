#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================

DAWVSCAudioProcessorEditor::DAWVSCAudioProcessorEditor(DAWVSCAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), commitListBoxModel(commitHistory)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);

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

    // Initialize buttons
    addAndMakeVisible(browseButton);
    addAndMakeVisible(checkoutButton);
    addAndMakeVisible(branchButton);
    addAndMakeVisible(deleteBranchButton);
    addAndMakeVisible(mergeButton);
    addAndMakeVisible(goForwardButton);

    // set button text
    if (projectPath.isNotEmpty())
	{
		browseButton.setButtonText("Branches");
	}
	else
	{
		browseButton.setButtonText("Browse...");
	}
    checkoutButton.setButtonText("Checkout");
    branchButton.setButtonText("Create Branch");
    deleteBranchButton.setButtonText("Delete branch");
    mergeButton.setButtonText("Merge");
    goForwardButton.setButtonText("Return");

    // on click events
    browseButton.onClick = [this] { browseButtonClicked(); };
    checkoutButton.onClick = [this] { checkoutButtonClicked(); };
    goForwardButton.onClick = [this] { goForwardButtonClicked(); };
    branchButton.onClick = [this] { branchButtonClicked(); };
    deleteBranchButton.onClick = [this] { deleteBranchButtonClicked(); };
    mergeButton.onClick = [this] { mergeButtonClicked(); };

    // set button bounds
    browseButton.setBounds(10, 10, getWidth() - 20, 20);
    checkoutButton.setBounds(10, 40, 100, 20);
    branchButton.setBounds(110, 40, 60, 20);
    deleteBranchButton.setBounds(170, 40, 60, 20);
    mergeButton.setBounds(230, 40, 60, 20);
    goForwardButton.setBounds(290, 40, 100, 20);

    // Initialize ListBox
    addAndMakeVisible(commitListBox);
    commitListBox.setModel(&commitListBoxModel);
    commitListBox.setBounds(10, 70, getWidth() - 20, getHeight() - 80);
    refreshCommitListBox();
    // Initialize callback for commit history changes
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
    if (projectPath.isEmpty())
	{
        // Selecting a project directory
        chooser = std::make_unique<juce::FileChooser>("Select project directory", juce::File::getSpecialLocation(juce::File::userHomeDirectory), "*");

        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
            [this](const juce::FileChooser& fc)
            {
                if (fc.getResult().exists())
                {
                    audioProcessor.setProjectPath(fc.getResult().getFullPathName());
                    audioProcessor.checkForGit(audioProcessor.getProjectPath());
                    refreshCommitListBox();
                    browseButton.setButtonText("Branches");
                }
            });
	}
	else
	{
        // Selecting a branch
        juce::StringArray branches = audioProcessor.getBranches();
		juce::PopupMenu m;
		for (int i = 0; i < branches.size(); i++)
		{
            if (branches[i].isEmpty()) continue;
			m.addItem(i + 1, branches[i]);
		}
        m.showMenuAsync(
            juce::PopupMenu::Options().withTargetComponent(&browseButton),
            [this, branches](int result)
            {
                if (result > 0) // Check if a valid menu item is selected
                {
                    juce::String branch = branches[result - 1];
                    if (branch.contains("*")) return; // Do not checkout the current branch
                    juce::String cmd = "git checkout " + branch;
                    executeAndRefresh(cmd);
                }
            }
        );
	}

}

void DAWVSCAudioProcessorEditor::checkoutButtonClicked()
{
    int row = commitListBox.getSelectedRow();
    juce::StringArray commitHistory = audioProcessor.getCommitHistory();
    if (row >= 0 && row < commitHistory.size())
	{
		juce::String hash = commitHashes[row];
        juce::String cmd = "git checkout " + hash;
		executeAndRefresh(cmd);
	}
}

void DAWVSCAudioProcessorEditor::goForwardButtonClicked()
{
    juce::String res = "";
    res = audioProcessor.executeCommand("git status");
    if (res.contains("HEAD detached at"))
    {
        // AT = just checking out a commit, we should return to master branch without worrying about any changes
        executeAndRefresh("git checkout master");
    }
}

void DAWVSCAudioProcessorEditor::branchButtonClicked()
{
    auto alertWindow = std::make_unique<juce::AlertWindow>("Create branch", "Enter branch name", juce::AlertWindow::NoIcon);
    alertWindow->addTextEditor("branchName", "", "Branch name:");
    alertWindow->addButton("Create", 1);
    alertWindow->addButton("Cancel", 0);
    alertWindow->enterModalState(true, juce::ModalCallbackFunction::create([this, alertWindow = alertWindow.get()](int result) mutable
        {
        if (result != 0)
        {
            juce::String branchName = alertWindow->getTextEditorContents("branchName");
            branchName = branchName.replaceCharacter(' ', '-');
            juce::String cmd = "git checkout -b " + branchName;
            executeAndRefresh(cmd);
        }
    }));

    this->alertWindow = std::move(alertWindow);

}

void DAWVSCAudioProcessorEditor::deleteBranchButtonClicked()
{
    juce::String currentBranch = audioProcessor.getCurrentBranch();
    if (currentBranch.contains("master"))
    {
        DBG("Cannot delete master branch");
    }
    else if (currentBranch.contains("HEAD detached at"))
    {
        DBG("Cannot delete detached HEAD");
        DAWVSCAudioProcessorEditor::goForwardButtonClicked();
    }
    else
    {
        juce::String cmd = "git checkout master && git branch -D " + currentBranch;
        executeAndRefresh(cmd);
    }
}

void DAWVSCAudioProcessorEditor::mergeButtonClicked()
{
    juce::String currentBranch = audioProcessor.getCurrentBranch();
    if (currentBranch == "")
    {
        DBG("Error: Likely in detached HEAD state, press 'Return' to go back to master branch");
    }
    else if (currentBranch.contains("master"))
	{
		DBG("Already on master branch");
	}
	else
	{
		executeAndRefresh("git checkout master && git merge " + currentBranch);
        juce::String cmd = "git branch -D " + currentBranch;
        audioProcessor.executeCommand(cmd.toStdString());
	}
}

void DAWVSCAudioProcessorEditor::refreshCommitListBox()
{
    // separate the hash from the rest of the commit message
    commitHashes.clear();
    commitHistory.clear();
    juce::StringArray commitHistoryTmp = audioProcessor.getCommitHistory();
    for (int i = 0; i < commitHistoryTmp.size(); i++)
	{
		commitHashes.add(commitHistoryTmp[i].upToFirstOccurrenceOf(" ", false, false));
        commitHistory.add(commitHistoryTmp[i].fromFirstOccurrenceOf(" ", false, false));
	}
    commitListBox.updateContent();
    commitListBox.selectRow(0);
}

void DAWVSCAudioProcessorEditor::executeAndRefresh(juce::String command)
{
    // Execute command and refresh the DAW
	audioProcessor.executeCommand(command.toStdString());
	audioProcessor.reloadWorkingTree();
}