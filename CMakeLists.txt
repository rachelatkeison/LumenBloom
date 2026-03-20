cmake_minimum_required(VERSION 3.15)

project(LumenBloom VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(LumenBloom
    COMPANY_NAME "Aureine Audio Systems"
    BUNDLE_ID com.aureine.lumenbloom
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE AuAS
    PLUGIN_CODE LuBl
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "LumenBloom"
)

juce_generate_juce_header(LumenBloom)

target_sources(LumenBloom
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/SynthSound.h
        Source/Oscillator.h
        Source/SynthVoice.h
)

target_compile_definitions(LumenBloom
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(LumenBloom
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)