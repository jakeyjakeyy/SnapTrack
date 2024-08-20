/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


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

void DAWVSCAudioProcessor::executeCommand(const char* command, juce::String& result)
{
    std::array<char, 128> buffer;
    std::string resultString;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command, "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        resultString += buffer.data();
    }
    DBG("Command: " + juce::String(command) + " Result: " + juce::String(resultString.c_str()));
    result.append(resultString, resultString.length());
}

void DAWVSCAudioProcessor::setProjectPath(const juce::String& path)
{
	projectPath = std::make_unique<juce::File>(path);
    if (projectPath->findChildFiles(juce::File::findFiles, true, "*").size() > 0) {
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

void DAWVSCAudioProcessor::checkForGit(const juce::String& path, juce::String& result)
{
    juce::File projectDir(path);
    juce::Array<juce::File> gitFolders;
    projectDir.findChildFiles(gitFolders, juce::File::findDirectories, false, ".git");
    juce::String resString = "";

    if (gitFolders.isEmpty())
    {
        DBG("Git repository not found, initializing git repository in " + path);
        resString = "Git repository not found, initializing git repository in " + path + "\n";
        result.append(resString, resString.length());
        DBG("Attempting initialization of git repository in " + path);
        executeCommand("git init", result);
        if (os.toLowerCase().contains("windows") || os.toLowerCase().contains("mac")) {
            resString = "Creating .gitignore for windows/mac\n";
            result.append(resString, resString.length());
            executeCommand("echo Backup/ > .gitignore && echo Ableton Project Info/ >> .gitignore", result);
        }
        else if (os.toLowerCase().contains("linux")) {
            resString = "Creating .gitignore for linux\n";
            result.append(resString, resString.length());
            executeCommand("sh -c 'echo Backup/ > .gitignore && echo \"Ableton Project Info/\" >> .gitignore'", result);
        }
        executeCommand("git add . && git commit -m \"Initial commit\"" , result);
        resString = "Git repository initialized\n";
        result.append(resString, resString.length());
        checkForGit(path, result);
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
	executeCommand("git --version", result);
    gitVersion = result;
	return gitVersion;
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DAWVSCAudioProcessor();
}
