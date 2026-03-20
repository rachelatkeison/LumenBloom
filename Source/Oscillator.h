#pragma once
#include <JuceHeader.h>

//==============================================================
// basic wave choices
//==============================================================
enum class WaveType
{
    sine = 0,
    square,
    saw
};

//==============================================================
// tiny oscillator class
// this is just phase + waveform math basically
//==============================================================
class Oscillator
{
public:
    Oscillator() = default;

    //==========================================================
    // setup sample rate
    //==========================================================
    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        updateAngleDelta();
    }

    //==========================================================
    // set waveform
    //==========================================================
    void setWaveType(WaveType newWaveType)
    {
        waveType = newWaveType;
    }

    //==========================================================
    // set pitch
    //==========================================================
    void setFrequency(float newFrequency)
    {
        frequency = newFrequency;
        updateAngleDelta();
    }

    //==========================================================
    // restart phase on new notes and stuff
    //==========================================================
    void reset()
    {
        currentAngle = 0.0;
    }

    //==========================================================
    // get one sample
    //==========================================================
    float getNextSample()
    {
        float sample = 0.0f;

        switch (waveType)
        {
            case WaveType::sine:
                sample = (float) std::sin(currentAngle);
                break;

            case WaveType::square:
                sample = (std::sin(currentAngle) >= 0.0) ? 1.0f : -1.0f;
                break;

            case WaveType::saw:
            {
                auto phase = currentAngle / juce::MathConstants<double>::twoPi;
                sample = (float)((2.0 * phase) - 1.0);
                break;
            }

            default:
                sample = 0.0f;
                break;
        }

        currentAngle += angleDelta;

        while (currentAngle >= juce::MathConstants<double>::twoPi)
            currentAngle -= juce::MathConstants<double>::twoPi;

        return sample;
    }

private:
    //==========================================================
    // this just updates how fast the phase moves
    //==========================================================
    void updateAngleDelta()
    {
        if (sampleRate > 0.0)
            angleDelta = (frequency / sampleRate) * juce::MathConstants<double>::twoPi;
    }

    double sampleRate = 44100.0;
    float frequency = 440.0f;

    double currentAngle = 0.0;
    double angleDelta = 0.0;

    WaveType waveType = WaveType::sine;
};