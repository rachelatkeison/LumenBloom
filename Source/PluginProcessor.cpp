#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================
// constructor
//==============================================================
LumenBloomAudioProcessor::LumenBloomAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                    #if !JucePlugin_IsMidiEffect
                     #if !JucePlugin_IsSynth
                      .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     #endif
                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                    #endif
                      ),
      apvts(*this, nullptr, "PARAMETERS", createParameters())
#endif
{
    for (int i = 0; i < 12; ++i)
        synth.addVoice(new SynthVoice());

    synth.addSound(new SynthSound());
}

LumenBloomAudioProcessor::~LumenBloomAudioProcessor()
{
}

//==============================================================
// plugin info
//==============================================================
const juce::String LumenBloomAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LumenBloomAudioProcessor::acceptsMidi() const   { return true; }
bool LumenBloomAudioProcessor::producesMidi() const  { return false; }
bool LumenBloomAudioProcessor::isMidiEffect() const  { return false; }
double LumenBloomAudioProcessor::getTailLengthSeconds() const { return 0.0; }

//==============================================================
// program stuff
//==============================================================
int LumenBloomAudioProcessor::getNumPrograms() { return 1; }
int LumenBloomAudioProcessor::getCurrentProgram() { return 0; }
void LumenBloomAudioProcessor::setCurrentProgram(int) {}
const juce::String LumenBloomAudioProcessor::getProgramName(int) { return {}; }
void LumenBloomAudioProcessor::changeProgramName(int, const juce::String&) {}

//==============================================================
// prepare
//==============================================================
void LumenBloomAudioProcessor::prepareToPlay(double sampleRate, int)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    updateVoiceParameters();
}

void LumenBloomAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LumenBloomAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
}
#endif

//==============================================================
// preset folder
//==============================================================
juce::File LumenBloomAudioProcessor::getPresetFolder() const
{
    auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                    .getChildFile("LumenBloom Presets");

    if (!folder.exists())
        folder.createDirectory();

    return folder;
}

//==============================================================
// preset names
//==============================================================
juce::StringArray LumenBloomAudioProcessor::getPresetNames() const
{
    juce::StringArray names;
    auto folder = getPresetFolder();

    auto files = folder.findChildFiles(juce::File::findFiles, false, "*.lbpreset");

    for (auto& file : files)
        names.add(file.getFileNameWithoutExtension());

    names.sort(true);
    return names;
}

//==============================================================
// save preset
//==============================================================
bool LumenBloomAudioProcessor::savePresetToFile(const juce::String& presetName)
{
    auto cleanName = presetName.trim();

    if (cleanName.isEmpty())
        return false;

    auto file = getPresetFolder().getChildFile(cleanName + ".lbpreset");

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    if (xml == nullptr)
        return false;

    return xml->writeTo(file);
}

//==============================================================
// load preset
//==============================================================
bool LumenBloomAudioProcessor::loadPresetFromFile(const juce::String& presetName)
{
    auto cleanName = presetName.trim();

    if (cleanName.isEmpty())
        return false;

    auto file = getPresetFolder().getChildFile(cleanName + ".lbpreset");

    if (!file.existsAsFile())
        return false;

    std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(file));

    if (xml == nullptr)
        return false;

    if (!xml->hasTagName(apvts.state.getType()))
        return false;

    apvts.replaceState(juce::ValueTree::fromXml(*xml));
    return true;
}

//==============================================================
// push params into voices
//==============================================================
void LumenBloomAudioProcessor::updateVoiceParameters()
{
    auto attack       = apvts.getRawParameterValue("ATTACK")->load();
    auto decay        = apvts.getRawParameterValue("DECAY")->load();
    auto sustain      = apvts.getRawParameterValue("SUSTAIN")->load();
    auto release      = apvts.getRawParameterValue("RELEASE")->load();

    auto wave1Choice  = (int) apvts.getRawParameterValue("WAVE1")->load();
    auto wave2Choice  = (int) apvts.getRawParameterValue("WAVE2")->load();
    auto oscMix       = apvts.getRawParameterValue("OSC_MIX")->load();
    auto detune       = apvts.getRawParameterValue("DETUNE")->load();
    auto noise        = apvts.getRawParameterValue("NOISE")->load();

    auto wtPos        = apvts.getRawParameterValue("WT_POS")->load();
    auto wtMix        = apvts.getRawParameterValue("WT_MIX")->load();

    auto filterModeChoice = (int) apvts.getRawParameterValue("FILTER_MODE")->load();
    auto cutoff       = apvts.getRawParameterValue("CUTOFF")->load();
    auto resonance    = apvts.getRawParameterValue("RESONANCE")->load();

    auto lfoRate      = apvts.getRawParameterValue("LFO_RATE")->load();
    auto lfoDepth     = apvts.getRawParameterValue("LFO_DEPTH")->load();

    auto modDestChoice = (int) apvts.getRawParameterValue("MOD_DEST")->load();
    auto modAmount     = apvts.getRawParameterValue("MOD_AMOUNT")->load();

    auto drive        = apvts.getRawParameterValue("DRIVE")->load();

    auto unisonChoice = (int) apvts.getRawParameterValue("UNISON")->load();
    auto width        = apvts.getRawParameterValue("WIDTH")->load();

    juce::ADSR::Parameters adsrParams;
    adsrParams.attack  = attack;
    adsrParams.decay   = decay;
    adsrParams.sustain = sustain;
    adsrParams.release = release;

    auto toWave = [](int value)
    {
        switch (value)
        {
            case 0: return WaveType::sine;
            case 1: return WaveType::square;
            case 2: return WaveType::saw;
            default: return WaveType::sine;
        }
    };

    auto toFilterMode = [](int value)
    {
        switch (value)
        {
            case 0: return FilterMode::lowPass;
            case 1: return FilterMode::highPass;
            case 2: return FilterMode::bandPass;
            default: return FilterMode::lowPass;
        }
    };

    auto toModDest = [](int value)
    {
        switch (value)
        {
            case 0: return ModDestination::cutoff;
            case 1: return ModDestination::wavetablePosition;
            case 2: return ModDestination::pitch;
            default: return ModDestination::cutoff;
        }
    };

    auto wave1 = toWave(wave1Choice);
    auto wave2 = toWave(wave2Choice);
    auto mode  = toFilterMode(filterModeChoice);
    auto modDest = toModDest(modDestChoice);

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->setOsc1WaveType(wave1);
            voice->setOsc2WaveType(wave2);
            voice->setOscMix(oscMix);
            voice->setDetuneCents(detune);
            voice->setNoiseLevel(noise);

            voice->setWavetablePosition(wtPos);
            voice->setWavetableMix(wtMix);

            voice->setADSRParameters(adsrParams);

            voice->setFilterMode(mode);
            voice->setFilterCutoff(cutoff);
            voice->setFilterResonance(resonance);

            voice->setLfoRate(lfoRate);
            voice->setLfoDepth(lfoDepth);

            voice->setModDestination(modDest);
            voice->setModAmount(modAmount);

            voice->setDrive(drive);

            voice->setUnisonCount(unisonChoice + 1);
            voice->setStereoWidth(width);
        }
    }
}

//==============================================================
// light output polish
//==============================================================
void LumenBloomAudioProcessor::applySoftClip(juce::AudioBuffer<float>& buffer)
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer(channel);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
            data[i] = std::tanh(data[i] * 1.10f);
    }
}

//==============================================================
// tiny global width tidy
//==============================================================
void LumenBloomAudioProcessor::applyWidthPolish(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() < 2)
        return;

    auto width = apvts.getRawParameterValue("WIDTH")->load();
    auto polish = 0.15f * width;

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        auto mid = 0.5f * (left[i] + right[i]);
        auto side = 0.5f * (left[i] - right[i]);

        side *= (1.0f + polish);

        left[i]  = mid + side;
        right[i] = mid - side;
    }
}

//==============================================================
// audio callback
//==============================================================
void LumenBloomAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();

    updateVoiceParameters();

    // combine host midi + on-screen keyboard midi
    juce::MidiBuffer combinedMidi;
    combinedMidi.addEvents(midiMessages, 0, buffer.getNumSamples(), 0);
    keyboardState.processNextMidiBuffer(combinedMidi, 0, buffer.getNumSamples(), true);

    synth.renderNextBlock(buffer, combinedMidi, 0, buffer.getNumSamples());

    applyWidthPolish(buffer);
    applySoftClip(buffer);

    auto gain = apvts.getRawParameterValue("GAIN")->load();
    buffer.applyGain(gain);
}

//==============================================================
// editor
//==============================================================
bool LumenBloomAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* LumenBloomAudioProcessor::createEditor()
{
    return new LumenBloomAudioProcessorEditor(*this);
}

//==============================================================
// state save/load
//==============================================================
void LumenBloomAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LumenBloomAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

//==============================================================
// params
//==============================================================
juce::AudioProcessorValueTreeState::ParameterLayout LumenBloomAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "WAVE1", "Wave 1",
        juce::StringArray { "Sine", "Square", "Saw" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "WAVE2", "Wave 2",
        juce::StringArray { "Sine", "Square", "Saw" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "OSC_MIX", "Osc Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.28f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DETUNE", "Detune",
        juce::NormalisableRange<float>(0.0f, 35.0f, 0.1f), 1.2f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "NOISE", "Noise",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "WT_POS", "WT Position",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.18f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "WT_MIX", "WT Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.28f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "ATTACK", "Attack",
        juce::NormalisableRange<float>(0.01f, 5.0f, 0.01f), 0.03f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DECAY", "Decay",
        juce::NormalisableRange<float>(0.01f, 5.0f, 0.01f), 0.35f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "SUSTAIN", "Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.78f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "RELEASE", "Release",
        juce::NormalisableRange<float>(0.01f, 6.0f, 0.01f), 0.60f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "FILTER_MODE", "Filter Mode",
        juce::StringArray { "LP", "HP", "BP" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "CUTOFF", "Cutoff",
        juce::NormalisableRange<float>(40.0f, 18000.0f, 1.0f, 0.3f), 1500.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "RESONANCE", "Resonance",
        juce::NormalisableRange<float>(0.1f, 1.5f, 0.01f), 0.24f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "LFO_RATE", "LFO Rate",
        juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f), 1.2f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "LFO_DEPTH", "LFO Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.10f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "MOD_DEST", "Mod Destination",
        juce::StringArray { "Cutoff", "WT Position", "Pitch" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "MOD_AMOUNT", "Mod Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.22f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DRIVE", "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.12f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "UNISON", "Unison",
        juce::StringArray { "1", "2", "3", "4" }, 1));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "WIDTH", "Width",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.55f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DELAY_TIME", "Delay Time",
        juce::NormalisableRange<float>(1.0f, 900.0f, 1.0f), 220.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DELAY_FB", "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 0.90f, 0.01f), 0.25f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DELAY_MIX", "Delay Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.12f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "REVERB_SIZE", "Reverb Size",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "REVERB_MIX", "Reverb Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.14f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "GAIN", "Gain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.72f));

    return { params.begin(), params.end() };
}

//==============================================================
// create plugin
//==============================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LumenBloomAudioProcessor();
}