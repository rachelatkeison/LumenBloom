#pragma once

#include <JuceHeader.h>
#include "SynthSound.h"
#include "SynthVoice.h"

//==============================================================
// main plugin processor
//==============================================================
class LumenBloomAudioProcessor : public juce::AudioProcessor
{
public:
    LumenBloomAudioProcessor();
    ~LumenBloomAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
   #endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // main parameter tree
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // preset helpers
    juce::File getPresetFolder() const;
    juce::StringArray getPresetNames() const;
    bool savePresetToFile(const juce::String& presetName);
    bool loadPresetFromFile(const juce::String& presetName);

    // keyboard state for on-screen keyboard + computer typing keys
    juce::MidiKeyboardState keyboardState;

private:
    void updateVoiceParameters();
    void applySoftClip(juce::AudioBuffer<float>& buffer);
    void applyWidthPolish(juce::AudioBuffer<float>& buffer);

    juce::Synthesiser synth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LumenBloomAudioProcessor)
};