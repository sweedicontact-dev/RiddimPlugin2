#pragma once
#include <juce_core/juce_core.h>
#include <vector>
#include <utility>

struct Preset
{
    juce::String name;
    std::vector<std::pair<juce::String, float>> values;   // lo que no está acá queda en default
};

// Índices: wave 0 Saw, 1 Square, 2 Sine, 3 Tri | filter 0 LP, 1 BP, 2 HP, 3 Notch
// lfo_div 0 1/1, 1 1/2, 2 1/4, 3 1/4T, 4 1/8, 5 1/8T, 6 1/16, 7 1/16T, 8 1/32
// lfo_shape 0 Sine, 1 Tri, 2 Saw Up, 3 Saw Down, 4 Square
inline const std::vector<Preset>& getFactoryPresets()
{
    static const std::vector<Preset> presets =
    {
        { "Init", {} },

        { "Riddim Wub 140", {
            {"osc1_wave",0},{"osc1_unison",3},{"osc1_detune",0.25f},{"osc1_level",0.8f},
            {"sub_level",0.45f},{"fm_amount",0.2f},{"osc2_level",0},{"fold",0.1f},{"drive",0.35f},
            {"filter_type",0},{"cutoff",180},{"res",0.55f},
            {"lfo_sync",1},{"lfo_div",2},{"lfo_shape",0},{"lfo_cutoff",0.75f},{"lfo_retrig",1},
            {"amp_d",300},{"amp_s",1},{"amp_r",60} } },

        { "Gollum Growl", {
            {"osc1_wave",1},{"osc1_unison",1},{"osc1_level",0.7f},
            {"osc2_wave",2},{"osc2_ratio",2},{"fm_amount",0.55f},{"lfo_fm",0.4f},
            {"sub_level",0.3f},{"fold",0.2f},{"drive",0.5f},
            {"filter_type",1},{"filter_24",0},{"cutoff",450},{"res",0.55f},
            {"lfo_div",4},{"lfo_shape",1},{"lfo_cutoff",0.45f},{"amp_r",50} } },

        { "Tripler Chop", {
            {"osc1_wave",0},{"osc1_unison",5},{"osc1_detune",0.35f},{"sub_level",0.5f},
            {"drive",0.4f},{"filter_type",0},{"cutoff",140},{"res",0.5f},
            {"lfo_div",5},{"lfo_shape",3},{"lfo_cutoff",0.8f},
            {"amp_d",200},{"amp_s",1},{"amp_r",60} } },

        { "Dojo Square Gate", {
            {"osc1_wave",1},{"osc1_unison",2},{"osc1_detune",0.15f},{"sub_level",0.5f},
            {"fold",0.15f},{"drive",0.5f},{"cutoff",120},{"res",0.35f},
            {"lfo_div",6},{"lfo_shape",4},{"lfo_cutoff",1},{"amp_r",40} } },

        { "Screech Lead", {
            {"osc1_wave",0},{"osc1_unison",5},{"osc1_detune",0.4f},{"osc1_level",0.7f},
            {"osc2_wave",2},{"osc2_ratio",3},{"fm_amount",0.7f},{"lfo_fm",0.3f},
            {"sub_level",0},{"fold",0.5f},{"lfo_fold",0.3f},{"drive",0.6f},
            {"filter_24",0},{"cutoff",3500},{"res",0.3f},{"lfo_cutoff",0},
            {"lfo_div",2},{"pitch_env",12},{"pitch_decay",60},{"glide",40},{"bend_range",12},
            {"amp_r",120} } },

        { "Sub Drop", {
            {"osc1_wave",2},{"osc1_level",0.3f},{"osc1_unison",1},{"sub_level",0.9f},
            {"drive",0.15f},{"cutoff",400},{"res",0.1f},{"lfo_cutoff",0},
            {"pitch_env",24},{"pitch_decay",120},
            {"amp_d",600},{"amp_s",0.8f},{"amp_r",150} } },

        { "Metal Fold Riddim", {
            {"osc1_wave",3},{"osc1_unison",3},{"osc1_detune",0.2f},{"sub_level",0.4f},
            {"fold",0.75f},{"lfo_fold",0.5f},{"drive",0.5f},
            {"cutoff",250},{"res",0.45f},{"lfo_div",3},{"lfo_shape",0},{"lfo_cutoff",0.6f} } },

        { "Vowel Yoi", {
            {"osc1_wave",0},{"osc1_unison",2},{"osc2_wave",1},{"osc2_ratio",2},{"osc2_level",0.2f},
            {"fm_amount",0.1f},{"drive",0.45f},{"filter_type",1},{"cutoff",380},{"res",0.7f},
            {"lfo_div",1},{"lfo_shape",2},{"lfo_cutoff",0.5f},{"sub_level",0.35f} } },

        { "Dirty Ghost", {
            {"osc1_wave",0},{"noise_level",0.25f},{"osc2_ratio",1.5f},{"fm_amount",0.3f},
            {"fold",0.3f},{"drive",0.7f},{"filter_type",1},{"filter_24",0},{"cutoff",900},{"res",0.4f},
            {"lfo_div",4},{"lfo_shape",0},{"lfo_cutoff",0.4f},{"sub_level",0.3f} } },

        { "Tearout Stab", {
            {"osc1_wave",0},{"osc1_unison",7},{"osc1_detune",0.5f},{"sub_level",0.3f},
            {"fold",0.2f},{"drive",0.6f},{"cutoff",600},{"res",0.4f},
            {"lfo_div",6},{"lfo_shape",1},{"lfo_cutoff",0.5f},
            {"pitch_env",7},{"pitch_decay",40},
            {"amp_d",150},{"amp_s",0},{"amp_r",80} } },
    };
    return presets;
}
