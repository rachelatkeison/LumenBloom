# LumenBloom

**Adaptive Synth Engine — Aureine Audio Systems**

LumenBloom is a polyphonic synthesizer plugin built in C++ using JUCE.  
It was designed as a flagship portfolio project combining audio DSP, sound design, and a minimal, expressive interface.

---

## 🎧 Demo

👉 [Watch Demo](PASTE_YOUR_VIDEO_LINK_HERE)

---

## ✨ Overview

This is a fully functional synth engine built from scratch — not just a basic oscillator demo.

The goal was to create something that feels:
- responsive
- atmospheric
- musically expressive

instead of purely technical.

---

## 🔊 Features

### Core DSP
- Polyphonic voice architecture  
- Dual oscillators (sine / square / saw)  
- Wavetable blending  
- ADSR envelope  
- Multimode filter (LP / HP / BP)  
- LFO modulation  
- Modulation routing (cutoff / wavetable / pitch)

### Sound Design
- Detune + unison voices  
- Noise layer  
- Stereo width control  
- Soft drive / saturation  
- Velocity-sensitive performance keyboard  

### Effects
- Delay  
- Reverb  
- Stereo widening  

### Plugin + System
- VST3 plugin support  
- Standalone application  
- Preset save / load system  
- MIDI input support  
- DAW automation (APVTS)

### Interface
- Custom JUCE UI  
- Waveform visualizer  
- Envelope visualizer  
- Interactive keyboard with glow feedback  

---

## 🧠 Signal Flow

MIDI → Voice → Oscillators → Wavetable → Envelope → Filter → Modulation → Effects → Output

Each note creates a voice that runs its own full signal chain in real time.

---

## 🛠️ Tech Stack

- C++  
- JUCE Framework  
- CMake  
- Xcode (macOS)

---

## 🚀 Build Instructions

```bash
git clone https://github.com/YOURUSERNAME/LumenBloom.git
cd LumenBloom
cmake -B build
cmake --build build
````

### Run

* Standalone:

```
build/LumenBloom_artefacts/Standalone/
```

* Plugin:
  Load the VST3 in your DAW (REAPER recommended)

---

## 🎨 Design Approach

LumenBloom was built to feel like an instrument, not just a plugin.

Focus areas:

* clean, minimal interface
* smooth interaction and motion
* expressive preset design
* balance between simplicity and depth

---

## 📸 Screenshots

*Add screenshots here*

---

## 📁 Project Structure

```
Source/
  PluginProcessor.*   // DSP + engine
  PluginEditor.*      // UI
  SynthVoice.*        // voice system
  Oscillator.*        // waveform generation
  SynthSound.*        // voice binding

JUCE/
CMakeLists.txt
```

---

## 💫 About

**Rachel Atkeison**
Computer Science + Music Technology

LumenBloom is part of my *Aureine Audio Systems* portfolio, focused on building expressive audio software and digital instruments.

---

## 📬 Contact

GitHub: [https://github.com/rachelatkeison](https://github.com/rachelatkeison)

```
