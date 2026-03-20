#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>
#include "SynthSound.h"
#include "Oscillator.h"
#include "WavetableOscillator.h"

//==============================================================
// filter modes
//==============================================================
enum class FilterMode
{
    lowPass = 0,
    highPass,
    bandPass
};

//==============================================================
// mod routing choices
//==============================================================
enum class ModDestination
{
    cutoff = 0,
    wavetablePosition,
    pitch
};

//==============================================================
// one voice = one note
// this version adds unison lanes + stereo spread
//==============================================================
class SynthVoice : public juce::SynthesiserVoice
{
public:
    static constexpr int maxUnisonVoices = 4;

    SynthVoice() = default;

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SynthSound*>(sound) != nullptr;
    }

    //==========================================================
    // note on
    //==========================================================
    void startNote(int midiNoteNumber,
                   float velocity,
                   juce::SynthesiserSound*,
                   int) override
    {
        baseFrequency = (float) juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        noteVelocity = velocity;

        lfo.reset();

        for (int i = 0; i < maxUnisonVoices; ++i)
        {
            osc1Voices[i].reset();
            osc2Voices[i].reset();
            wavetableVoices[i].reset();
        }

        updateAllLaneFrequencies(baseFrequency);
        updateLanePans();

        adsr.noteOn();
    }

    //==========================================================
    // note off
    //==============================================================
    void stopNote(float, bool allowTailOff) override
    {
        adsr.noteOff();

        if (!allowTailOff || !adsr.isActive())
            clearCurrentNote();
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    //==========================================================
    // setup
    //==========================================================
    void setCurrentPlaybackSampleRate(double newRate) override
    {
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);

        lfo.setSampleRate(newRate);
        adsr.setSampleRate(newRate);

        for (int i = 0; i < maxUnisonVoices; ++i)
        {
            osc1Voices[i].setSampleRate(newRate);
            osc2Voices[i].setSampleRate(newRate);
            wavetableVoices[i].setSampleRate(newRate);
            wavetableVoices[i].setPosition(wavetablePosition);
        }

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = newRate;
        spec.maximumBlockSize = 512;
        spec.numChannels = 1;

        filter.reset();
        filter.prepare(spec);
        updateFilterType();
        filter.setCutoffFrequency(filterCutoff);
        filter.setResonance(filterResonance);
    }

    //==========================================================
    // parameter setters
    //==========================================================
    void setOsc1WaveType(WaveType type)
    {
        for (auto& osc : osc1Voices)
            osc.setWaveType(type);
    }

    void setOsc2WaveType(WaveType type)
    {
        for (auto& osc : osc2Voices)
            osc.setWaveType(type);
    }

    void setOscMix(float newMix)         { oscMix = juce::jlimit(0.0f, 1.0f, newMix); }
    void setDetuneCents(float newDetune) { detuneCents = newDetune; }
    void setNoiseLevel(float newNoise)   { noiseLevel = juce::jlimit(0.0f, 1.0f, newNoise); }

    void setWavetablePosition(float newPos)
    {
        wavetablePosition = juce::jlimit(0.0f, 1.0f, newPos);

        for (auto& wt : wavetableVoices)
            wt.setPosition(wavetablePosition);
    }

    void setWavetableMix(float newMix)
    {
        wavetableMix = juce::jlimit(0.0f, 1.0f, newMix);
    }

    void setADSRParameters(const juce::ADSR::Parameters& newParams)
    {
        adsr.setParameters(newParams);
    }

    void setFilterMode(FilterMode newMode)
    {
        filterMode = newMode;
        updateFilterType();
    }

    void setFilterCutoff(float newCutoff)
    {
        filterCutoff = newCutoff;
    }

    void setFilterResonance(float newRes)
    {
        filterResonance = juce::jlimit(0.1f, 1.5f, newRes);
    }

    void setLfoRate(float newRate)
    {
        lfo.setFrequency(newRate);
    }

    void setLfoDepth(float newDepth)
    {
        lfoDepth = juce::jlimit(0.0f, 1.0f, newDepth);
    }

    void setModDestination(ModDestination newDest)
    {
        modDestination = newDest;
    }

    void setModAmount(float newAmount)
    {
        modAmount = juce::jlimit(0.0f, 1.0f, newAmount);
    }

    void setDrive(float newDrive)
    {
        drive = juce::jlimit(0.0f, 1.0f, newDrive);
    }

    void setUnisonCount(int newCount)
    {
        unisonCount = juce::jlimit(1, maxUnisonVoices, newCount);
        updateLanePans();
    }

    void setStereoWidth(float newWidth)
    {
        stereoWidth = juce::jlimit(0.0f, 1.0f, newWidth);
        updateLanePans();
    }

    //==========================================================
    // render
    //==========================================================
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample,
                         int numSamples) override
    {
        if (!isVoiceActive())
            return;

        auto numChannels = outputBuffer.getNumChannels();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            auto env = adsr.getNextSample();

            auto lfoBipolar = lfo.getNextSample();
            auto lfoUnipolar = (lfoBipolar * 0.5f) + 0.5f;

            auto workingFrequency = baseFrequency;
            auto workingCutoff = filterCutoff;
            auto workingWtPos = wavetablePosition;

            if (modDestination == ModDestination::pitch)
            {
                auto pitchRatio = 1.0f + (lfoBipolar * modAmount * 0.03f);
                workingFrequency = baseFrequency * pitchRatio;
            }
            else if (modDestination == ModDestination::cutoff)
            {
                workingCutoff = filterCutoff * (1.0f + (lfoUnipolar * modAmount * 2.2f));
            }
            else if (modDestination == ModDestination::wavetablePosition)
            {
                workingWtPos = juce::jlimit(0.0f, 1.0f, wavetablePosition + (lfoBipolar * modAmount * 0.45f));
            }

            workingCutoff = juce::jlimit(40.0f, 18000.0f, workingCutoff);

            updateAllLaneFrequencies(workingFrequency);

            for (int i = 0; i < unisonCount; ++i)
                wavetableVoices[i].setPosition(workingWtPos);

            filter.setCutoffFrequency(workingCutoff);
            filter.setResonance(filterResonance);

            float leftSum = 0.0f;
            float rightSum = 0.0f;

            for (int lane = 0; lane < unisonCount; ++lane)
            {
                auto osc1Sample = osc1Voices[lane].getNextSample();
                auto osc2Sample = osc2Voices[lane].getNextSample();
                auto wtSample   = wavetableVoices[lane].getNextSample();

                auto baseBlend = ((1.0f - oscMix) * osc1Sample) + (oscMix * osc2Sample * 0.85f);
                auto sourceSample = ((1.0f - wavetableMix) * baseBlend) + (wavetableMix * wtSample);

                auto noiseSample = (random.nextFloat() * 2.0f - 1.0f) * noiseLevel * 0.05f;
                auto rawSample = sourceSample + noiseSample;

                auto filteredSample = filter.processSample(0, rawSample);
                auto drivenSample = std::tanh(filteredSample * (1.0f + drive * 4.0f));

                auto laneSample = drivenSample / (float) juce::jmax(1, unisonCount);

                leftSum  += laneSample * laneLeftGain[(size_t) lane];
                rightSum += laneSample * laneRightGain[(size_t) lane];
            }

            auto finalLeft  = leftSum  * env * noteVelocity * 0.30f;
            auto finalRight = rightSum * env * noteVelocity * 0.30f;

            if (numChannels >= 2)
            {
                outputBuffer.addSample(0, startSample + sample, finalLeft);
                outputBuffer.addSample(1, startSample + sample, finalRight);
            }
            else if (numChannels == 1)
            {
                outputBuffer.addSample(0, startSample + sample, 0.5f * (finalLeft + finalRight));
            }
        }

        if (!adsr.isActive())
            clearCurrentNote();
    }

private:
    //==========================================================
    // update all lane frequencies
    //==========================================================
    void updateAllLaneFrequencies(float workingFrequency)
    {
        if (workingFrequency <= 0.0f)
            return;

        for (int lane = 0; lane < maxUnisonVoices; ++lane)
        {
            auto laneSpread = getLaneDetuneOffset(lane);
            auto laneRatio = std::pow(2.0f, laneSpread / 1200.0f);

            auto freq = workingFrequency * laneRatio;

            osc1Voices[(size_t) lane].setFrequency(freq);

            auto osc2Ratio = std::pow(2.0f, detuneCents / 1200.0f);
            osc2Voices[(size_t) lane].setFrequency(freq * osc2Ratio);

            wavetableVoices[(size_t) lane].setFrequency(freq);
        }
    }

    //==========================================================
    // small detune pattern for unison lanes
    //==========================================================
    float getLaneDetuneOffset(int lane) const
    {
        switch (lane)
        {
            case 0: return -0.75f * detuneCents;
            case 1: return  0.75f * detuneCents;
            case 2: return -1.50f * detuneCents;
            case 3: return  1.50f * detuneCents;
            default: return 0.0f;
        }
    }

    //==========================================================
    // update lane pans from width
    //==========================================================
    void updateLanePans()
    {
        laneLeftGain.fill(1.0f);
        laneRightGain.fill(1.0f);

        if (unisonCount <= 1)
            return;

        for (int lane = 0; lane < unisonCount; ++lane)
        {
            auto normalized = (unisonCount == 1) ? 0.0f
                                                 : juce::jmap((float) lane,
                                                              0.0f,
                                                              (float) (unisonCount - 1),
                                                              -1.0f,
                                                              1.0f);

            auto pan = normalized * stereoWidth;
            auto angle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;

            laneLeftGain[(size_t) lane]  = std::cos(angle);
            laneRightGain[(size_t) lane] = std::sin(angle);
        }
    }

    //==========================================================
    // filter type
    //==========================================================
    void updateFilterType()
    {
        switch (filterMode)
        {
            case FilterMode::lowPass:
                filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
                break;

            case FilterMode::highPass:
                filter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
                break;

            case FilterMode::bandPass:
                filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
                break;

            default:
                filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
                break;
        }
    }

    std::array<Oscillator, maxUnisonVoices> osc1Voices;
    std::array<Oscillator, maxUnisonVoices> osc2Voices;
    std::array<WavetableOscillator, maxUnisonVoices> wavetableVoices;

    Oscillator lfo;

    juce::ADSR adsr;
    juce::Random random;
    juce::dsp::StateVariableTPTFilter<float> filter;

    std::array<float, maxUnisonVoices> laneLeftGain  { 1.0f, 1.0f, 1.0f, 1.0f };
    std::array<float, maxUnisonVoices> laneRightGain { 1.0f, 1.0f, 1.0f, 1.0f };

    float baseFrequency = 440.0f;
    float noteVelocity = 1.0f;

    float oscMix = 0.28f;
    float detuneCents = 1.2f;
    float noiseLevel = 0.0f;

    float wavetablePosition = 0.18f;
    float wavetableMix = 0.28f;

    FilterMode filterMode = FilterMode::lowPass;
    float filterCutoff = 1500.0f;
    float filterResonance = 0.24f;

    float lfoDepth = 0.10f;
    ModDestination modDestination = ModDestination::cutoff;
    float modAmount = 0.22f;

    float drive = 0.12f;

    int unisonCount = 2;
    float stereoWidth = 0.55f;
};