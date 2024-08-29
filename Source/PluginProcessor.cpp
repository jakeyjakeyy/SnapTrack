/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <Windows.h>


//==============================================================================
DAWVSCAudioProcessor::DAWVSCAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

DAWVSCAudioProcessor::~DAWVSCAudioProcessor()
{
}

//==============================================================================

const juce::String DAWVSCAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DAWVSCAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DAWVSCAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DAWVSCAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DAWVSCAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DAWVSCAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DAWVSCAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DAWVSCAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DAWVSCAudioProcessor::getProgramName (int index)
{
    return {};
}

void DAWVSCAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DAWVSCAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void DAWVSCAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DAWVSCAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DAWVSCAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================
bool DAWVSCAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DAWVSCAudioProcessor::createEditor()
{
    createdEditor = true;
    return new DAWVSCAudioProcessorEditor (*this);
}

//==============================================================================
void DAWVSCAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    DBG("getStateInformation() called");
    juce::XmlElement xml("DAWVSCAudioProcessorState");
    if (projectPath != nullptr) {
		xml.setAttribute("projectPath", projectPath->getFullPathName());
	}

    // Add any other metadata here

    // Convert the XML to a string and then store it in the memory block
    copyXmlToBinary(xml, destData);
}

void DAWVSCAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    DBG("setStateInformation() called");
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr)
    {
        if (xmlState->hasAttribute("projectPath"))
        {
            setProjectPath(xmlState->getStringAttribute("projectPath"));
        }
    }
    // Restore any other parameters from the xmlState here
}

juce::String DAWVSCAudioProcessor::executeCommand(const std::string& command)
{
    if (!createdEditor) {
        createEditor();
    }
    if (os.toLowerCase().contains("windows") || os.toLowerCase().contains("mac")) {
        HANDLE hPipeRead, hPipeWrite;
        SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
        saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
        saAttr.lpSecurityDescriptor = NULL;

        // Create a pipe to get results from child's stdout.
        if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
            return "Error creating pipe";

        // Ensure the read handle to the pipe for STDOUT is not inherited.
        SetHandleInformation(hPipeRead, HANDLE_FLAG_INHERIT, 0);

        PROCESS_INFORMATION processInfo;
        STARTUPINFO startupInfo;
        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;
        startupInfo.hStdOutput = hPipeWrite;
        startupInfo.hStdError = hPipeWrite;
        startupInfo.hStdInput = NULL; // Ensure the input handle is not inherited

        std::string cmd = "cmd /C " + command;

        if (!CreateProcess(NULL, const_cast<char*>(cmd.c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInfo))
        {
            CloseHandle(hPipeWrite);
            CloseHandle(hPipeRead);
            return "Error creating process";
        }

        // Close the write end of the pipe before reading from the read end of the pipe.
        CloseHandle(hPipeWrite);

        char buffer[128];
        DWORD bytesRead;
        std::string result;

        // Read output from the child process.
        while (ReadFile(hPipeRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            result += buffer;
        }

        CloseHandle(hPipeRead);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);

        return result;
    }
    else {
        std::array<char, 128> buffer;
        std::string cmd = command + " 2>&1"; // Capture both stdout and stderr
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
        juce::String result;

        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result.append(buffer.data(), buffer.size());
        }

        return result;
    }
}

void DAWVSCAudioProcessor::setProjectPath(const juce::String& path)
{
	projectPath = std::make_unique<juce::File>(path);
    if (projectPath->exists()) {
		projectPath->setAsCurrentWorkingDirectory();
	} else {
		projectPath = nullptr;
	}
}

juce::String DAWVSCAudioProcessor::getProjectPath()
{
    if (projectPath == nullptr) {
		return "";
	}
	return projectPath->getFullPathName();
}

void DAWVSCAudioProcessor::checkForGit(const juce::String& path)
{
    juce::File projectDir(path);
    juce::Array<juce::File> gitFolders;
    projectDir.findChildFiles(gitFolders, juce::File::findDirectories, false, ".git");
    juce::String resString = "";
    juce::String result = "";

    if (gitFolders.isEmpty())
    {
        DBG("Git repository not found, initializing git repository in " + path);
        resString = "Git repository not found, initializing git repository in " + path + "\n";
        result.append(resString, resString.length());
        DBG("Attempting initialization of git repository in " + path);
        executeCommand("git init");
        if (os.toLowerCase().contains("windows") || os.toLowerCase().contains("mac")) {
            resString = "Creating .gitignore for windows/mac\n";
            result.append(resString, resString.length());
            executeCommand("echo Backup/ > .gitignore && echo Ableton Project Info/ >> .gitignore");
        }
        else if (os.toLowerCase().contains("linux")) {
            resString = "Creating .gitignore for linux\n";
            result.append(resString, resString.length());
            executeCommand("sh -c 'echo Backup/ > .gitignore && echo \"Ableton Project Info/\" >> .gitignore'");
        }
        resString = "Git repository initialized\n";
        result.append(resString, resString.length());
        checkForGit(path);
    }
    else
    {
        resString = "gitFolders is not empty\n";
        result.append(resString, resString.length());
    }
}

juce::String DAWVSCAudioProcessor::getOS()
{
    os = juce::SystemStats::getOperatingSystemName();
    return os;
}

juce::String DAWVSCAudioProcessor::getGitVersion()
{
	juce::String result;
	executeCommand("git --version");
    gitVersion = result;
	return gitVersion;
}

void DAWVSCAudioProcessor::checkGitStatus()
{
    juce::String result;
    result = executeCommand("git status --porcelain");

    if (result != "")
    {
        DBG("Working tree has changed");
        juce::String status = executeCommand("git status");
        if (status.contains("HEAD detached"))
		{
            juce::String hash = status.fromFirstOccurrenceOf("HEAD detached ", false, true);
            hash = hash.fromFirstOccurrenceOf(" ", false, true);
            hash = hash.upToFirstOccurrenceOf("\n", false, true);
            juce::String cmd = "git checkout -b " + hash + "-branch";
            executeCommand(cmd.toStdString());
        }
        else
        {
            executeCommand("git add . && git commit -m \"Auto commit\"");
            commitHistoryChangedCallback();
        }
    }
}

void DAWVSCAudioProcessor::reloadWorkingTree()
{
    if (projectPath != nullptr) 
    {
        juce::Array<juce::File> children;
        projectPath->findChildFiles(children, juce::File::findFiles, false, "*");
        for (auto child : children)
		{
			if (child.getFileExtension() == ".als" || child.getFileExtension() == ".flp") 
			{
                child.startAsProcess();
			}
		}
    }
}

juce::StringArray DAWVSCAudioProcessor::getCommitHistory()
{
	juce::String result;
	result = executeCommand("git log --pretty=format:\"%h %s %ar\"");
	juce::StringArray commits;
	commits.addLines(result);
    // Remove the first commit, which is the most recent commit
    // Removing this line cleans up the commit history list, but looks confusing if a user
    // expects the most recent commit to be at the top of the list
    // commits.remove(0);
	return commits;
}

void DAWVSCAudioProcessor::setCommitHistoryChangedCallback(CommitHistoryChangedCallback callback)
{
	commitHistoryChangedCallback = std::move(callback);
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DAWVSCAudioProcessor();
}

juce::String DAWVSCAudioProcessor::getCurrentBranch()
{
	juce::String result;
	result = executeCommand("git branch --show-current");
	return result;
}

juce::StringArray DAWVSCAudioProcessor::getBranches()
{
    juce::String result;
	result = executeCommand("git branch");
	juce::StringArray branches;
	branches.addLines(result);
	return branches;
}