# See JUCE cmake example at https://github.com/juce-framework/JUCE/blob/master/examples/CMake/AudioPlugin/CMakeLists.txt
cmake_minimum_required(VERSION 3.19)

set(name DDS19)
set(model "model/dds19_lstm32.json")

project(${name} VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

include(cpm/CPM.cmake)
CPMAddPackage("gh:juce-framework/JUCE#master")
CPMAddPackage("gh:fmtlib/fmt#master")
CPMAddPackage(
        NAME nlohmann_json
        GITHUB_REPOSITORY nlohmann/json
        VERSION 3.10.2)

find_package (Eigen3 REQUIRED NO_MODULE)

juce_add_plugin("${name}"
        COMPANY_NAME Velbloudek
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Vb77
        PLUGIN_CODE Dds1
        FORMATS AU VST3
        PRODUCT_NAME ${name})

target_sources(${name}
    PRIVATE
        src/Processor.cpp
        src/Editor.cpp
        src/DelayLine.cpp
        src/Model.cpp)

target_compile_definitions(${name}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

juce_add_binary_data(model_lib SOURCES ${model})

target_link_libraries(${name}
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        model_lib
        fmt
        nlohmann_json
        Eigen3::Eigen
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)