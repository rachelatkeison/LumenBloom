#include "PluginEditor.h"

//==============================================================
// constructor
//==============================================================
LumenBloomAudioProcessorEditor::LumenBloomAudioProcessorEditor(LumenBloomAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      waveformView(p),
      envelopeView(p),
      keyboardComponent(audioProcessor.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setSize(1500, 880);

    titleLabel.setText("LumenBloom", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setFont(juce::Font(juce::FontOptions(38.0f, juce::Font::bold)));
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Aureine Audio Systems", juce::dontSendNotification);
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    subtitleLabel.setFont(juce::Font(juce::FontOptions(14.0f)));
    addAndMakeVisible(subtitleLabel);

    lineLabel.setText("adaptive synth engine", juce::dontSendNotification);
    lineLabel.setJustificationType(juce::Justification::centredLeft);
    lineLabel.setFont(juce::Font(juce::FontOptions(12.5f)));
    addAndMakeVisible(lineLabel);

    presetFolderLabel.setText(audioProcessor.getPresetFolder().getFullPathName(), juce::dontSendNotification);
    presetFolderLabel.setJustificationType(juce::Justification::centredLeft);
    presetFolderLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    addAndMakeVisible(presetFolderLabel);

    octaveLabel.setJustificationType(juce::Justification::centred);
    octaveLabel.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
    addAndMakeVisible(octaveLabel);

    styleBox(presetBox);
    addAndMakeVisible(presetBox);

    styleButton(savePresetButton);
    styleButton(loadPresetButton);
    styleButton(refreshPresetButton);
    styleButton(octaveDownButton);
    styleButton(octaveUpButton);

    savePresetButton.setButtonText("Save");
    loadPresetButton.setButtonText("Load");
    refreshPresetButton.setButtonText("Refresh");
    octaveDownButton.setButtonText("Oct -");
    octaveUpButton.setButtonText("Oct +");

    savePresetButton.onClick = [this]()
    {
        auto name = makeTimestampPresetName();
        audioProcessor.savePresetToFile(name);
        refreshPresetList();
        presetBox.setText(name, juce::dontSendNotification);
    };

    loadPresetButton.onClick = [this]()
    {
        auto name = presetBox.getText().trim();
        if (name.isNotEmpty())
            audioProcessor.loadPresetFromFile(name);
    };

    refreshPresetButton.onClick = [this]()
    {
        refreshPresetList();
    };

    octaveDownButton.onClick = [this]()
    {
        keyboardBaseOctave = juce::jlimit(0, 8, keyboardBaseOctave - 1);
        keyboardComponent.setKeyPressBaseOctave(keyboardBaseOctave);
        updateOctaveLabel();
    };

    octaveUpButton.onClick = [this]()
    {
        keyboardBaseOctave = juce::jlimit(0, 8, keyboardBaseOctave + 1);
        keyboardComponent.setKeyPressBaseOctave(keyboardBaseOctave);
        updateOctaveLabel();
    };

    styleBox(wave1Box);
    styleBox(wave2Box);
    styleBox(filterModeBox);
    styleBox(modDestBox);
    styleBox(unisonBox);

    wave1Box.addItem("Sine", 1);
    wave1Box.addItem("Square", 2);
    wave1Box.addItem("Saw", 3);

    wave2Box.addItem("Sine", 1);
    wave2Box.addItem("Square", 2);
    wave2Box.addItem("Saw", 3);

    filterModeBox.addItem("LP", 1);
    filterModeBox.addItem("HP", 2);
    filterModeBox.addItem("BP", 3);

    modDestBox.addItem("Cutoff", 1);
    modDestBox.addItem("WT Position", 2);
    modDestBox.addItem("Pitch", 3);

    unisonBox.addItem("1", 1);
    unisonBox.addItem("2", 2);
    unisonBox.addItem("3", 3);
    unisonBox.addItem("4", 4);

    wave1Attachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "WAVE1", wave1Box);
    wave2Attachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "WAVE2", wave2Box);
    filterModeAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "FILTER_MODE", filterModeBox);
    modDestAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "MOD_DEST", modDestBox);
    unisonAttachment = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "UNISON", unisonBox);

    styleKnob(oscMixSlider);
    styleKnob(detuneSlider);
    styleKnob(noiseSlider);

    styleKnob(wtPosSlider);
    styleKnob(wtMixSlider);

    styleKnob(attackSlider);
    styleKnob(decaySlider);
    styleKnob(sustainSlider);
    styleKnob(releaseSlider);

    styleKnob(cutoffSlider);
    styleKnob(resonanceSlider);

    styleKnob(lfoRateSlider);
    styleKnob(lfoDepthSlider);
    styleKnob(modAmountSlider);

    styleKnob(driveSlider);
    styleKnob(widthSlider);
    styleKnob(gainSlider);

    oscMixAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "OSC_MIX", oscMixSlider);
    detuneAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DETUNE", detuneSlider);
    noiseAttachment  = std::make_unique<SliderAttachment>(audioProcessor.apvts, "NOISE", noiseSlider);

    wtPosAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "WT_POS", wtPosSlider);
    wtMixAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "WT_MIX", wtMixSlider);

    attackAttachment  = std::make_unique<SliderAttachment>(audioProcessor.apvts, "ATTACK", attackSlider);
    decayAttachment   = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DECAY", decaySlider);
    sustainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "SUSTAIN", sustainSlider);
    releaseAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "RELEASE", releaseSlider);

    cutoffAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "CUTOFF", cutoffSlider);
    resonanceAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "RESONANCE", resonanceSlider);

    lfoRateAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "LFO_RATE", lfoRateSlider);
    lfoDepthAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "LFO_DEPTH", lfoDepthSlider);
    modAmountAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "MOD_AMOUNT", modAmountSlider);

    driveAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DRIVE", driveSlider);
    widthAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "WIDTH", widthSlider);
    gainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "GAIN", gainSlider);

    addAndMakeVisible(waveformView);
    addAndMakeVisible(envelopeView);

    // keyboard setup
    keyboardComponent.setColour(juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour::fromRGB(245, 240, 255));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour::fromRGB(40, 32, 58));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour::fromRGB(80, 70, 120));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour::fromRGB(200, 180, 255).withAlpha(0.40f));
    keyboardComponent.setColour(juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour::fromRGB(180, 140, 255).withAlpha(0.62f));
    keyboardComponent.setAvailableRange(24, 96);
    keyboardComponent.setKeyWidth(24.0f);
    keyboardComponent.setOctaveForMiddleC(keyboardBaseOctave);
    keyboardComponent.setKeyPressBaseOctave(keyboardBaseOctave);
    keyboardComponent.setVelocity(1.0f, true); // mouse height changes velocity
    keyboardComponent.setWantsKeyboardFocus(true);
    addAndMakeVisible(keyboardComponent);

    audioProcessor.keyboardState.addListener(this);
    updateOctaveLabel();
    refreshPresetList();
    startTimerHz(30);
}

LumenBloomAudioProcessorEditor::~LumenBloomAudioProcessorEditor()
{
    audioProcessor.keyboardState.removeListener(this);
}

//==============================================================
// style helpers
//==============================================================
void LumenBloomAudioProcessorEditor::styleKnob(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGB(205, 184, 255));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour::fromRGB(236, 230, 248));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour::fromRGB(242, 238, 248));
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB(24, 20, 36));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(slider);
}

void LumenBloomAudioProcessorEditor::styleBox(juce::ComboBox& box)
{
    box.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(24, 20, 36));
    box.setColour(juce::ComboBox::textColourId, juce::Colour::fromRGB(242, 238, 248));
    box.setColour(juce::ComboBox::outlineColourId, juce::Colour::fromRGB(108, 92, 156));
    box.setColour(juce::ComboBox::arrowColourId, juce::Colour::fromRGB(210, 190, 255));
    addAndMakeVisible(box);
}

void LumenBloomAudioProcessorEditor::styleButton(juce::TextButton& button)
{
    button.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(38, 31, 56));
    button.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(90, 72, 135));
    button.setColour(juce::TextButton::textColourOffId, juce::Colour::fromRGB(241, 237, 248));
    addAndMakeVisible(button);
}

//==============================================================
// draw helpers
//==============================================================
void LumenBloomAudioProcessorEditor::drawPanel(juce::Graphics& g, juce::Rectangle<int> area, const juce::String& title)
{
    auto panelFill = juce::Colour::fromRGB(20, 17, 30);
    auto panelEdge = juce::Colour::fromRGB(112, 96, 164);
    auto titleText = juce::Colour::fromRGB(236, 231, 246);

    g.setColour(panelFill);
    g.fillRoundedRectangle(area.toFloat(), 18.0f);

    g.setColour(panelEdge.withAlpha(0.30f));
    g.drawRoundedRectangle(area.toFloat(), 18.0f, 1.6f);

    g.setColour(titleText);
    g.setFont(juce::Font(juce::FontOptions(16.0f)));
    g.drawText(title, area.removeFromTop(34).reduced(14, 4), juce::Justification::centredLeft);
}

void LumenBloomAudioProcessorEditor::drawKnobLabel(juce::Graphics& g, const juce::String& text, juce::Rectangle<int> area)
{
    g.setColour(juce::Colour::fromRGB(188, 179, 209));
    g.setFont(juce::Font(juce::FontOptions(13.0f)));
    g.drawFittedText(text, area, juce::Justification::centred, 1);
}

void LumenBloomAudioProcessorEditor::refreshPresetList()
{
    auto currentText = presetBox.getText();
    auto names = audioProcessor.getPresetNames();

    presetBox.clear();

    for (int i = 0; i < names.size(); ++i)
        presetBox.addItem(names[i], i + 1);

    if (currentText.isNotEmpty() && names.contains(currentText))
        presetBox.setText(currentText, juce::dontSendNotification);
    else if (names.size() > 0)
        presetBox.setSelectedId(1, juce::dontSendNotification);
}

juce::String LumenBloomAudioProcessorEditor::makeTimestampPresetName() const
{
    auto now = juce::Time::getCurrentTime();
    return "Preset_" + now.formatted("%Y%m%d_%H%M%S");
}

void LumenBloomAudioProcessorEditor::updateOctaveLabel()
{
    octaveLabel.setText("Octave " + juce::String(keyboardBaseOctave), juce::dontSendNotification);
}

//==============================================================
// timer + keyboard glow
//==============================================================
void LumenBloomAudioProcessorEditor::timerCallback()
{
    glowTarget = (activeKeyboardNotes > 0) ? 1.0f : 0.0f;
    glowCurrent += (glowTarget - glowCurrent) * 0.18f;
    repaint();
}

void LumenBloomAudioProcessorEditor::handleNoteOn(juce::MidiKeyboardState*, int, int, float)
{
    ++activeKeyboardNotes;
}

void LumenBloomAudioProcessorEditor::handleNoteOff(juce::MidiKeyboardState*, int, int, float)
{
    activeKeyboardNotes = juce::jmax(0, activeKeyboardNotes - 1);
}

//==============================================================
// paint
//==============================================================
void LumenBloomAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bg         = juce::Colour::fromRGB(9, 8, 14);
    auto shell      = juce::Colour::fromRGB(16, 13, 24);
    auto shellEdge  = juce::Colour::fromRGB(140, 120, 190);
    auto accentGlow = juce::Colour::fromRGB(195, 170, 255);
    auto text       = juce::Colour::fromRGB(241, 237, 248);

    g.fillAll(bg);

    auto bounds = getLocalBounds().reduced(18);

    g.setColour(shell);
    g.fillRoundedRectangle(bounds.toFloat(), 22.0f);

    g.setColour(shellEdge.withAlpha(0.25f));
    g.drawRoundedRectangle(bounds.toFloat(), 22.0f, 2.0f);

    g.setColour(accentGlow.withAlpha(0.15f));
    g.fillRoundedRectangle(juce::Rectangle<float>(30.0f, 82.0f, getWidth() - 60.0f, 2.5f), 1.2f);

    juce::Rectangle<int> presetPanel(30,   110, 1440, 88);
    juce::Rectangle<int> visualPanel(30,   214, 1440, 170);
    juce::Rectangle<int> oscPanel   (30,   400, 250, 320);
    juce::Rectangle<int> wtPanel    (295,  400, 210, 320);
    juce::Rectangle<int> envPanel   (520,  400, 290, 320);
    juce::Rectangle<int> modPanel   (825,  400, 250, 320);
    juce::Rectangle<int> stereoPanel(1090, 400, 380, 320);
    juce::Rectangle<int> keyboardPanel(30, 736, 1440, 114);

    drawPanel(g, presetPanel, "Presets");
    drawPanel(g, visualPanel, "Visualizers");
    drawPanel(g, oscPanel, "Oscillators");
    drawPanel(g, wtPanel,  "Wavetable");
    drawPanel(g, envPanel, "Envelope / Filter");
    drawPanel(g, modPanel, "Modulation");
    drawPanel(g, stereoPanel, "Unison / Stereo");
    drawPanel(g, keyboardPanel, "Performance Keyboard");

    // soft keyboard glow
    auto kbGlow = keyboardComponent.getBounds().toFloat().expanded(10.0f);
    g.setColour(accentGlow.withAlpha(0.08f + (0.18f * glowCurrent)));
    g.fillRoundedRectangle(kbGlow, 16.0f);

    // osc labels
    drawKnobLabel(g, "Wave 1", juce::Rectangle<int>(48, 435, 90, 18));
    drawKnobLabel(g, "Wave 2", juce::Rectangle<int>(172, 435, 90, 18));
    drawKnobLabel(g, "Mix", juce::Rectangle<int>(48, 525, 70, 18));
    drawKnobLabel(g, "Detune", juce::Rectangle<int>(120, 525, 80, 18));
    drawKnobLabel(g, "Noise", juce::Rectangle<int>(198, 525, 70, 18));

    // wt labels
    drawKnobLabel(g, "WT Pos", juce::Rectangle<int>(318, 435, 80, 18));
    drawKnobLabel(g, "WT Mix", juce::Rectangle<int>(405, 435, 80, 18));

    // env/filter labels
    drawKnobLabel(g, "Attack",  juce::Rectangle<int>(540, 435, 65, 18));
    drawKnobLabel(g, "Decay",   juce::Rectangle<int>(610, 435, 65, 18));
    drawKnobLabel(g, "Sustain", juce::Rectangle<int>(680, 435, 65, 18));
    drawKnobLabel(g, "Release", juce::Rectangle<int>(750, 435, 65, 18));

    drawKnobLabel(g, "Mode", juce::Rectangle<int>(545, 590, 80, 18));
    drawKnobLabel(g, "Cutoff", juce::Rectangle<int>(620, 590, 80, 18));
    drawKnobLabel(g, "Res", juce::Rectangle<int>(715, 590, 70, 18));

    // mod labels
    drawKnobLabel(g, "LFO Rate", juce::Rectangle<int>(848, 435, 80, 18));
    drawKnobLabel(g, "LFO Depth", juce::Rectangle<int>(948, 435, 80, 18));
    drawKnobLabel(g, "Dest", juce::Rectangle<int>(850, 590, 90, 18));
    drawKnobLabel(g, "Amount", juce::Rectangle<int>(955, 590, 75, 18));

    // stereo labels
    drawKnobLabel(g, "Drive", juce::Rectangle<int>(1110, 435, 70, 18));
    drawKnobLabel(g, "Unison", juce::Rectangle<int>(1195, 435, 80, 18));
    drawKnobLabel(g, "Width", juce::Rectangle<int>(1295, 435, 70, 18));
    drawKnobLabel(g, "Gain", juce::Rectangle<int>(1210, 590, 70, 18));

    g.setColour(text.withAlpha(0.70f));
    g.setFont(juce::Font(juce::FontOptions(12.0f)));
    g.drawFittedText("keyboard-ready instrument / click notes or use typing keys", 36, 854, 420, 18, juce::Justification::left, 1);
}

//==============================================================
// resized
//==============================================================
void LumenBloomAudioProcessorEditor::resized()
{
    titleLabel.setBounds(34, 24, 260, 32);
    subtitleLabel.setBounds(35, 56, 220, 18);
    lineLabel.setBounds(36, 86, 280, 16);

    presetBox.setBounds(52, 152, 280, 28);
    savePresetButton.setBounds(350, 152, 80, 28);
    loadPresetButton.setBounds(440, 152, 80, 28);
    refreshPresetButton.setBounds(530, 152, 90, 28);
    presetFolderLabel.setBounds(640, 152, 780, 26);

    waveformView.setBounds(48, 252, 900, 105);
    envelopeView.setBounds(970, 252, 480, 105);

    wave1Box.setBounds(48, 458, 90, 28);
    wave2Box.setBounds(172, 458, 90, 28);

    oscMixSlider.setBounds(46, 545, 70, 90);
    detuneSlider.setBounds(122, 545, 70, 90);
    noiseSlider.setBounds(198, 545, 70, 90);

    wtPosSlider.setBounds(318, 458, 75, 95);
    wtMixSlider.setBounds(405, 458, 75, 95);

    attackSlider.setBounds(538, 458, 60, 90);
    decaySlider.setBounds(608, 458, 60, 90);
    sustainSlider.setBounds(678, 458, 60, 90);
    releaseSlider.setBounds(748, 458, 60, 90);

    filterModeBox.setBounds(548, 615, 80, 28);
    cutoffSlider.setBounds(620, 615, 75, 95);
    resonanceSlider.setBounds(710, 615, 75, 95);

    lfoRateSlider.setBounds(845, 458, 75, 95);
    lfoDepthSlider.setBounds(945, 458, 75, 95);
    modDestBox.setBounds(850, 615, 90, 28);
    modAmountSlider.setBounds(955, 615, 75, 95);

    driveSlider.setBounds(1105, 458, 70, 95);
    unisonBox.setBounds(1190, 458, 90, 28);
    widthSlider.setBounds(1290, 458, 70, 95);
    gainSlider.setBounds(1210, 615, 75, 95);

    octaveDownButton.setBounds(54, 785, 76, 28);
    octaveLabel.setBounds(140, 785, 90, 28);
    octaveUpButton.setBounds(240, 785, 76, 28);

    keyboardComponent.setBounds(340, 770, 1100, 60);
}