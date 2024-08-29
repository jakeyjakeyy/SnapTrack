#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================

DAWVSCAudioProcessorEditor::DAWVSCAudioProcessorEditor(DAWVSCAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), commitListBoxModel(commitHistory), branchListBoxModel(branchList)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);

    // Get project path
    projectPath = audioProcessor.getProjectPath();
    DBG("Project path: " + projectPath);
    resString = "Project path: " + projectPath + "\n";
    result.append(resString, resString.length());

    if (projectPath.isNotEmpty())
    {
        audioProcessor.checkForGit(projectPath); // Check for git repository in project path
        addAndMakeVisible(commitListBox);
        addAndMakeVisible(commitButton);
        addAndMakeVisible(checkoutButton);
        addAndMakeVisible(goForwardButton);
        addAndMakeVisible(branchListBox);
        addAndMakeVisible(branchButton);
        addAndMakeVisible(mergeButton);
        addAndMakeVisible(deleteBranchButton);
    }
    else {
        addAndMakeVisible(browseButton);
    }

    // Initialize Browse
    browseButton.setButtonText("Browse...");
    browseButton.onClick = [this] { browseButtonClicked(); };
    browseButton.setBounds(100, 75, 200, 150);

    // Commits Controls
    commitListBox.setModel(&commitListBoxModel);
    commitListBox.setBounds(130, 5, 260, 180);
    commitButton.setBounds(130, commitListBox.getBottom() + 5, 260, 45);
    checkoutButton.setBounds(130, commitButton.getBottom(), 130, 45);
    goForwardButton.setBounds(checkoutButton.getRight(), commitButton.getBottom(), 130, 45);
    commitButton.setButtonText("Take a Snapshot");
    checkoutButton.setButtonText("Checkout");
    goForwardButton.setButtonText("Return");
    refreshCommitListBox();
    audioProcessor.setCommitHistoryChangedCallback([this] { refreshCommitListBox(); }); // Initialize callback for commit history changes
    checkoutButton.onClick = [this] { checkoutButtonClicked(); };
    goForwardButton.onClick = [this] { goForwardButtonClicked(); };

    // Branch Controls
    branchListBox.setModel(&branchListBoxModel);
    branchListBox.setBounds(10, 5, 110, 180);
    branchButton.setBounds(10, branchListBox.getBottom() + 5, 110, 45);
    mergeButton.setBounds(10, branchButton.getBottom(), 55, 45);
    deleteBranchButton.setBounds(mergeButton.getRight(), branchButton.getBottom(), 55, 45);
    branchButton.setButtonText("Create Branch");
    mergeButton.setButtonText("Merge");
    deleteBranchButton.setButtonText("Delete");
    branchButton.onClick = [this] { branchButtonClicked(); };
    mergeButton.onClick = [this] { mergeButtonClicked(); };
    deleteBranchButton.onClick = [this] { deleteBranchButtonClicked(); };

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
                addAndMakeVisible(branchListBox);
                addAndMakeVisible(branchButton);
                addAndMakeVisible(mergeButton);
                addAndMakeVisible(deleteBranchButton);
                addAndMakeVisible(commitListBox);
                addAndMakeVisible(commitButton);
                addAndMakeVisible(checkoutButton);
                addAndMakeVisible(goForwardButton);
                browseButton.setVisible(false);
            }
        });

}
    // use this for the branch viewer somewhere maybe idk
	//else
	//{
 //       // Selecting a branch
 //       juce::StringArray branches = audioProcessor.getBranches();
	//	juce::PopupMenu m;
	//	for (int i = 0; i < branches.size(); i++)
	//	{
 //           if (branches[i].isEmpty()) continue;
	//		m.addItem(i + 1, branches[i]);
	//	}
 //       m.showMenuAsync(
 //           juce::PopupMenu::Options().withTargetComponent(&browseButton),
 //           [this, branches](int result)
 //           {
 //               if (result > 0) // Check if a valid menu item is selected
 //               {
 //                   juce::String branch = branches[result - 1];
 //                   if (branch.contains("*")) return; // Do not checkout the current branch
 //                   juce::String cmd = "git checkout " + branch;
 //                   executeAndRefresh(cmd);
 //               }
 //           }
 //       );
	//}

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
        this->alertWindow.reset();
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