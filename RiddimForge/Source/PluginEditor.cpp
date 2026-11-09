#include "PluginEditor.h"

static constexpr int cellW = 92, cellH = 96, rowH = 124, topH = 64, margin = 14;

void RiddimEditor::add (int row, const char* id, const char* text, Kind kind)
{
    auto c = std::make_unique<Ctl>();
    c->kind = kind;
    c->row = row;
    c->label.setText (text, juce::dontSendNotification);
    c->label.setJustificationType (juce::Justification::centred);
    c->label.setColour (juce::Label::textColourId, juce::Colour (0xffc9ccd6));
    addAndMakeVisible (c->label);

    auto& apvts = proc.apvts;
    if (kind == Kind::Knob)
    {
        c->slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        c->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 16);
        c->slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff9dff3a));
        addAndMakeVisible (c->slider);
        c->sa = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, id, c->slider);
    }
    else if (kind == Kind::Choice)
    {
        if (auto* cp = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (id)))
            c->combo.addItemList (cp->choices, 1);
        addAndMakeVisible (c->combo);
        c->ca = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, id, c->combo);
    }
    else
    {
        c->toggle.setButtonText ("On");
        addAndMakeVisible (c->toggle);
        c->ba = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, id, c->toggle);
    }
    ctls.push_back (std::move (c));
}

RiddimEditor::RiddimEditor (RiddimProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    title.setText ("RIDDIMFORGE", juce::dontSendNotification);
    title.setFont (juce::FontOptions (26.f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, juce::Colour (0xff9dff3a));
    addAndMakeVisible (title);

    const auto& presets = getFactoryPresets();
    for (size_t i = 0; i < presets.size(); ++i) presetBox.addItem (presets[i].name, (int) i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { proc.applyPreset (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    using K = Kind;
    // Fila 0: osciladores
    add (0, "osc1_wave", "Osc1", K::Choice);  add (0, "osc1_unison", "Unison", K::Knob);
    add (0, "osc1_detune", "Detune", K::Knob); add (0, "osc1_level", "Osc1 Lvl", K::Knob);
    add (0, "osc2_wave", "Osc2", K::Choice);  add (0, "osc2_ratio", "Ratio", K::Knob);
    add (0, "osc2_level", "Osc2 Lvl", K::Knob); add (0, "sub_level", "Sub", K::Knob); add (0, "noise_level", "Noise", K::Knob);
    // Fila 1
    add (1, "fm_amount", "FM", K::Knob); add (1, "fold", "Fold", K::Knob); add (1, "drive", "Drive", K::Knob);
    add (1, "filter_type", "Filtro", K::Choice); add (1, "filter_24", "24 dB", K::Toggle);
    add (1, "cutoff", "Cutoff", K::Knob); add (1, "res", "Reso", K::Knob);
    // Fila 2
    add (2, "lfo_sync", "Sync", K::Toggle); add (2, "lfo_div", "Rate", K::Choice); add (2, "lfo_hz", "Rate Hz", K::Knob);
    add (2, "lfo_shape", "Forma", K::Choice); add (2, "lfo_retrig", "Retrig", K::Toggle);
    add (2, "lfo_cutoff", "> Cutoff", K::Knob); add (2, "lfo_fm", "> FM", K::Knob);
    add (2, "lfo_fold", "> Fold", K::Knob); add (2, "lfo_pitch", "> Pitch", K::Knob);
    // Fila 3
    add (3, "pitch_env", "Pitch Env", K::Knob); add (3, "pitch_decay", "P.Decay", K::Knob);
    add (3, "glide", "Glide", K::Knob); add (3, "bend_range", "Bend", K::Knob);
    add (3, "amp_a", "Attack", K::Knob); add (3, "amp_d", "Decay", K::Knob);
    add (3, "amp_s", "Sustain", K::Knob); add (3, "amp_r", "Release", K::Knob); add (3, "master", "Master", K::Knob);

    setSize (margin * 2 + cellW * 9, topH + rowH * 4 + margin);
}

void RiddimEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12141a));
    g.setFont (juce::FontOptions (12.f, juce::Font::bold));
    for (int r = 0; r < 4; ++r)
    {
        const int y = topH + r * rowH;
        g.setColour (juce::Colour (0xff1d2029));
        g.fillRoundedRectangle ((float) margin - 6, (float) y, (float) getWidth() - 2 * (margin - 6), (float) rowH - 6, 8.f);
        g.setColour (juce::Colour (0xff9dff3a).withAlpha (0.8f));
        g.drawText (rowNames[r], margin, y + 2, 400, 16, juce::Justification::left);
    }
}

void RiddimEditor::resized()
{
    title.setBounds (margin, 10, 260, 40);
    presetBox.setBounds (getWidth() - margin - 260, 16, 260, 30);

    int col[4] = { 0, 0, 0, 0 };
    for (auto& c : ctls)
    {
        const int x = margin + col[c->row]++ * cellW;
        const int y = topH + c->row * rowH + 18;
        c->label.setBounds (x, y, cellW, 16);
        const auto area = juce::Rectangle<int> (x, y + 16, cellW, cellH - 16);
        if (c->kind == Kind::Knob)        c->slider.setBounds (area);
        else if (c->kind == Kind::Choice) c->combo.setBounds (area.withSizeKeepingCentre (cellW - 10, 26));
        else                              c->toggle.setBounds (area.withSizeKeepingCentre (cellW - 10, 26));
    }
}
