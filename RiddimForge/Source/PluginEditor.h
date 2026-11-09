#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class RiddimEditor : public juce::AudioProcessorEditor
{
public:
    explicit RiddimEditor (RiddimProcessor&);
    ~RiddimEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    enum class Kind { Knob, Choice, Toggle };
    struct Ctl
    {
        Kind kind;
        int row;
        juce::Label label;
        juce::Slider slider;
        juce::ComboBox combo;
        juce::ToggleButton toggle;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   sa;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> ca;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   ba;
    };

    void add (int row, const char* id, const char* text, Kind kind);

    RiddimProcessor& proc;
    juce::ComboBox presetBox;
    juce::Label title;
    std::vector<std::unique_ptr<Ctl>> ctls;
    juce::StringArray rowNames { "OSCILADORES", "FM / FOLD / DRIVE / FILTRO", "LFO (WOBBLE)", "PITCH / ENVOLVENTE / MASTER" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RiddimEditor)
};
