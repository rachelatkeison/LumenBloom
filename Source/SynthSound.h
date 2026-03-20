#pragma once
#include <JuceHeader.h>

//==============================================================
// this is still tiny because it just says
// "yeah this synth can play all notes/channels"
//==============================================================
class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override  { return true; }
    bool appliesToChannel(int) override { return true; }
};