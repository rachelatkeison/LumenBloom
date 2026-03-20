```md
# LumenBloom

a soft, expressive synthesizer  
built as part of my aureine audio systems portfolio  

---

## what this is

LumenBloom is a polyphonic synthesizer plugin I built in C++ using JUCE.  

I wanted to create something that felt less like a technical tool and more like an instrument you could actually sit with — something that feels responsive, slightly dreamy, and alive when you play it.

This project is where I started seriously combining:
- audio DSP
- plugin development
- sound design
- visual interaction

into one cohesive system.

---

## demo

[watch the demo](PASTE_YOUR_VIDEO_LINK_HERE)

---

## what it can do

this is a full synth engine, not just a basic oscillator demo.

### sound engine
- polyphonic voice system  
- two oscillators (sine / square / saw)  
- wavetable blending  
- ADSR envelope  
- multimode filter (low pass / high pass / band pass)  
- LFO modulation  
- modulation routing (filter, wavetable, pitch)

### sound design features
- detune + unison voices  
- noise layer  
- stereo width control  
- soft drive / saturation  
- velocity-sensitive keyboard  

### effects
- delay  
- reverb  
- stereo widening  

### plugin + system features
- VST3 plugin  
- standalone app  
- preset save / load system  
- MIDI input support  
- parameter automation through APVTS  

### interface
- custom UI built in JUCE  
- waveform visualizer  
- envelope visualizer  
- interactive keyboard with glow feedback  

---

## how it works (simplified)

```

midi → voices → oscillators → wavetable → envelope → filter → modulation → effects → output

````

each note creates a voice, and each voice runs its own full signal chain in real time.

---

## technical notes

- written in C++ using JUCE  
- built with CMake + Xcode (macOS)  
- real-time safe (no memory allocation in the audio thread)  
- modular structure (DSP separated from UI)  

a lot of the focus was making sure everything stayed stable and responsive while still sounding full.

---

## building it

if you want to run it:

```bash
git clone https://github.com/YOURUSERNAME/LumenBloom.git
cd LumenBloom
cmake -B build
cmake --build build
````

standalone app will be in:

```
build/LumenBloom_artefacts/Standalone/
```

or load the VST3 in a DAW (I used REAPER while developing)

---

## design approach

I didn’t want this to feel like a generic plugin.

the goal was something that feels:

* soft but responsive
* minimal but expressive
* simple on the surface, deeper when you explore it

a lot of the choices (visuals, motion, presets) were made with that in mind.

---

## screenshots

(add screenshots here later)

---

## structure

```
Source/
  PluginProcessor.*   // audio + DSP engine
  PluginEditor.*      // UI
  SynthVoice.*        // voice system
  Oscillator.*        // waveform generation
  SynthSound.*        // voice binding

JUCE/
CMakeLists.txt
```

---

## about me

Rachel Atkeison
computer science student + music technology builder

this is part of my aureine audio systems portfolio where I’m exploring audio software, DSP, and interactive music tools.

---

## contact

github: [https://github.com/YOURUSERNAME](https://github.com/YOURUSERNAME)

````

