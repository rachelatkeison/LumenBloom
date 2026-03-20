#pragma once
#include <JuceHeader.h>
#include <array>
#include <cmath>

//==============================================================
// small wavetable osc
// 3 frames for now so we can morph between them
//==============================================================
class WavetableOscillator
{
public:
    static constexpr int tableSize = 2048;
    static constexpr int numFrames = 3;

    WavetableOscillator()
    {
        buildTables();
    }

    //==========================================================
    // setup
    //==========================================================
    void setSampleRate(double newSampleRate)
    {
        sampleRate = newSampleRate;
        updatePhaseDelta();
    }

    void setFrequency(float newFrequency)
    {
        frequency = newFrequency;
        updatePhaseDelta();
    }

    void setPosition(float newPosition)
    {
        position = juce::jlimit(0.0f, 1.0f, newPosition);
    }

    void reset()
    {
        phase = 0.0;
    }

    //==========================================================
    // one sample out
    //==========================================================
    float getNextSample()
    {
        auto frameFloat = position * (float) (numFrames - 1);
        auto frameA = juce::jlimit(0, numFrames - 1, (int) std::floor(frameFloat));
        auto frameB = juce::jlimit(0, numFrames - 1, frameA + 1);
        auto frameMix = frameFloat - (float) frameA;

        auto sampleA = getTableSample(frameA);
        auto sampleB = getTableSample(frameB);

        auto out = juce::jmap(frameMix, sampleA, sampleB);

        phase += phaseDelta;
        while (phase >= (double) tableSize)
            phase -= (double) tableSize;

        return out;
    }

private:
    //==========================================================
    // make a few nice-ish tables
    //==========================================================
    void buildTables()
    {
        for (int i = 0; i < tableSize; ++i)
        {
            auto t = (float) i / (float) tableSize;
            auto angle = t * juce::MathConstants<float>::twoPi;

            // frame 0 = soft sine-ish
            tables[0][i] = std::sin(angle);

            // frame 1 = glassier / brighter
            tables[1][i] =
                0.70f * std::sin(angle) +
                0.20f * std::sin(angle * 2.0f) +
                0.10f * std::sin(angle * 3.0f);

            // frame 2 = more airy / shimmering
            tables[2][i] =
                0.55f * std::sin(angle) +
                0.20f * std::sin(angle * 2.0f) +
                0.12f * std::sin(angle * 4.0f) +
                0.08f * std::sin(angle * 6.0f) +
                0.05f * std::sin(angle * 8.0f);
        }

        // normalize each frame
        for (int frame = 0; frame < numFrames; ++frame)
        {
            float maxMag = 0.0f;

            for (int i = 0; i < tableSize; ++i)
                maxMag = juce::jmax(maxMag, std::abs(tables[frame][i]));

            if (maxMag > 0.0f)
            {
                for (int i = 0; i < tableSize; ++i)
                    tables[frame][i] /= maxMag;
            }
        }
    }

    //==========================================================
    // read from one frame with simple linear interp
    //==========================================================
    float getTableSample(int frame) const
    {
        auto indexA = (int) phase;
        auto indexB = (indexA + 1) % tableSize;
        auto frac = (float) (phase - (double) indexA);

        auto a = tables[frame][indexA];
        auto b = tables[frame][indexB];

        return juce::jmap(frac, a, b);
    }

    void updatePhaseDelta()
    {
        if (sampleRate > 0.0)
            phaseDelta = ((double) frequency / sampleRate) * (double) tableSize;
    }

    std::array<std::array<float, tableSize>, numFrames> tables {};

    double sampleRate = 44100.0;
    double phase = 0.0;
    double phaseDelta = 0.0;

    float frequency = 440.0f;
    float position = 0.0f;
};