#pragma once

#include <JuceHeader.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

//==============================================================
// combined waveform viewer
//==============================================================
class WaveformView : public juce::Component,
                     private juce::Timer
{
public:
    explicit WaveformView(LumenBloomAudioProcessor& p) : processor(p)
    {
        startTimerHz(24);
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat().reduced(8.0f);

        g.setColour(juce::Colour::fromRGB(18, 16, 28));
        g.fillRoundedRectangle(area, 14.0f);

        g.setColour(juce::Colour::fromRGB(105, 92, 150).withAlpha(0.35f));
        g.drawRoundedRectangle(area, 14.0f, 1.2f);

        juce::Path path;
        auto midY = area.getCentreY();
        auto width = area.getWidth();

        path.startNewSubPath(area.getX(), midY);

        auto wave1 = (int) processor.apvts.getRawParameterValue("WAVE1")->load();
        auto wave2 = (int) processor.apvts.getRawParameterValue("WAVE2")->load();
        auto oscMix = processor.apvts.getRawParameterValue("OSC_MIX")->load();
        auto wtMix = processor.apvts.getRawParameterValue("WT_MIX")->load();
        auto wtPos = processor.apvts.getRawParameterValue("WT_POS")->load();

        for (int i = 0; i < 180; ++i)
        {
            auto t = (float) i / 179.0f;
            auto x = area.getX() + t * width;

            auto sample1 = getWaveSample(wave1, t);
            auto sample2 = getWaveSample(wave2, t);
            auto baseMix = juce::jmap(oscMix, sample1, sample2);
            auto wtSample = getWavetableRef(t, wtPos);
            auto sample = juce::jmap(wtMix, baseMix, wtSample);

            auto y = midY - sample * 28.0f;
            path.lineTo(x, y);
        }

        g.setColour(juce::Colour::fromRGB(205, 184, 255).withAlpha(0.95f));
        g.strokePath(path, juce::PathStrokeType(2.5f));
    }

private:
    LumenBloomAudioProcessor& processor;

    void timerCallback() override
    {
        repaint();
    }

    float getWaveSample(int wave, float t) const
    {
        auto angle = t * juce::MathConstants<float>::twoPi;

        switch (wave)
        {
            case 0: return std::sin(angle);
            case 1: return std::sin(angle) >= 0.0f ? 1.0f : -1.0f;
            case 2: return (2.0f * t) - 1.0f;
            default: return std::sin(angle);
        }
    }

    float getWavetableRef(float t, float wtPos) const
    {
        auto angle = t * juce::MathConstants<float>::twoPi;

        auto a = std::sin(angle);
        auto b = 0.70f * std::sin(angle) + 0.20f * std::sin(angle * 2.0f) + 0.10f * std::sin(angle * 3.0f);
        auto c = 0.55f * std::sin(angle) + 0.20f * std::sin(angle * 2.0f) + 0.12f * std::sin(angle * 4.0f);

        if (wtPos < 0.5f)
            return juce::jmap(wtPos * 2.0f, a, b);

        return juce::jmap((wtPos - 0.5f) * 2.0f, b, c);
    }
};

//==============================================================
// envelope view
//==============================================================
class EnvelopeView : public juce::Component,
                     private juce::Timer
{
public:
    explicit EnvelopeView(LumenBloomAudioProcessor& p) : processor(p)
    {
        startTimerHz(24);
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat().reduced(8.0f);

        g.setColour(juce::Colour::fromRGB(18, 16, 28));
        g.fillRoundedRectangle(area, 14.0f);

        g.setColour(juce::Colour::fromRGB(105, 92, 150).withAlpha(0.35f));
        g.drawRoundedRectangle(area, 14.0f, 1.2f);

        auto attack  = processor.apvts.getRawParameterValue("ATTACK")->load();
        auto decay   = processor.apvts.getRawParameterValue("DECAY")->load();
        auto sustain = processor.apvts.getRawParameterValue("SUSTAIN")->load();
        auto release = processor.apvts.getRawParameterValue("RELEASE")->load();

        auto total = attack + decay + release + 0.6f;
        auto w = area.getWidth();
        auto h = area.getHeight();

        auto x0 = area.getX();
        auto y0 = area.getBottom();

        auto xA = x0 + (attack / total) * w;
        auto xD = xA + (decay / total) * w;
        auto xS = xD + 0.25f * w;
        auto xR = juce::jmin(area.getRight(), xS + (release / total) * w);

        auto yTop = area.getY() + 8.0f;
        auto ySus = area.getBottom() - (sustain * (h - 16.0f));

        juce::Path env;
        env.startNewSubPath(x0, y0);
        env.lineTo(xA, yTop);
        env.lineTo(xD, ySus);
        env.lineTo(xS, ySus);
        env.lineTo(xR, y0);

        g.setColour(juce::Colour::fromRGB(191, 240, 255).withAlpha(0.88f));
        g.strokePath(env, juce::PathStrokeType(2.3f));
    }

private:
    LumenBloomAudioProcessor& processor;

    void timerCallback() override
    {
        repaint();
    }
};

//==============================================================
// custom aureine ui
//==============================================================
class LumenBloomAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer,
                                       private juce::MidiKeyboardStateListener
{
public:
    LumenBloomAudioProcessorEditor(LumenBloomAudioProcessor&);
    ~LumenBloomAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    LumenBloomAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label lineLabel;
    juce::Label presetFolderLabel;
    juce::Label octaveLabel;

    juce::ComboBox presetBox;
    juce::TextButton savePresetButton;
    juce::TextButton loadPresetButton;
    juce::TextButton refreshPresetButton;

    juce::ComboBox wave1Box;
    juce::ComboBox wave2Box;
    juce::ComboBox filterModeBox;
    juce::ComboBox modDestBox;
    juce::ComboBox unisonBox;

    juce::Slider oscMixSlider;
    juce::Slider detuneSlider;
    juce::Slider noiseSlider;

    juce::Slider wtPosSlider;
    juce::Slider wtMixSlider;

    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;

    juce::Slider cutoffSlider;
    juce::Slider resonanceSlider;

    juce::Slider lfoRateSlider;
    juce::Slider lfoDepthSlider;
    juce::Slider modAmountSlider;

    juce::Slider driveSlider;
    juce::Slider widthSlider;
    juce::Slider gainSlider;

    juce::TextButton octaveDownButton;
    juce::TextButton octaveUpButton;

    WaveformView waveformView;
    EnvelopeView envelopeView;

    juce::MidiKeyboardComponent keyboardComponent;

    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<ComboBoxAttachment> wave1Attachment;
    std::unique_ptr<ComboBoxAttachment> wave2Attachment;
    std::unique_ptr<ComboBoxAttachment> filterModeAttachment;
    std::unique_ptr<ComboBoxAttachment> modDestAttachment;
    std::unique_ptr<ComboBoxAttachment> unisonAttachment;

    std::unique_ptr<SliderAttachment> oscMixAttachment;
    std::unique_ptr<SliderAttachment> detuneAttachment;
    std::unique_ptr<SliderAttachment> noiseAttachment;

    std::unique_ptr<SliderAttachment> wtPosAttachment;
    std::unique_ptr<SliderAttachment> wtMixAttachment;

    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;

    std::unique_ptr<SliderAttachment> cutoffAttachment;
    std::unique_ptr<SliderAttachment> resonanceAttachment;

    std::unique_ptr<SliderAttachment> lfoRateAttachment;
    std::unique_ptr<SliderAttachment> lfoDepthAttachment;
    std::unique_ptr<SliderAttachment> modAmountAttachment;

    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> widthAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;

    int keyboardBaseOctave = 4;
    int activeKeyboardNotes = 0;
    float glowTarget = 0.0f;
    float glowCurrent = 0.0f;

    void styleKnob(juce::Slider& slider);
    void styleBox(juce::ComboBox& box);
    void styleButton(juce::TextButton& button);

    void drawPanel(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& title);
    void drawKnobLabel(juce::Graphics& g, const juce::String& text, juce::Rectangle<int> area);

    void refreshPresetList();
    juce::String makeTimestampPresetName() const;
    void updateOctaveLabel();

    void timerCallback() override;
    void handleNoteOn(juce::MidiKeyboardState*, int, int, float) override;
    void handleNoteOff(juce::MidiKeyboardState*, int, int, float) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LumenBloomAudioProcessorEditor)
};