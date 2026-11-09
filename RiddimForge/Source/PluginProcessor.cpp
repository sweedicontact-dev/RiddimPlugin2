#include "PluginProcessor.h"
#include "PluginEditor.h"

using Float  = juce::AudioParameterFloat;
using Choice = juce::AudioParameterChoice;
using Bool   = juce::AudioParameterBool;
using Int    = juce::AudioParameterInt;
using Range  = juce::NormalisableRange<float>;

juce::AudioProcessorValueTreeState::ParameterLayout RiddimProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    auto pid = [] (const char* id) { return juce::ParameterID { id, 1 }; };
    const juce::StringArray waves { "Saw", "Square", "Sine", "Tri" };

    // OSC
    p.push_back (std::make_unique<Choice> (pid ("osc1_wave"),   "Osc1 Wave", waves, 0));
    p.push_back (std::make_unique<Int>    (pid ("osc1_unison"), "Unison", 1, 7, 1));
    p.push_back (std::make_unique<Float>  (pid ("osc1_detune"), "Detune", Range (0.f, 1.f, 0.001f), 0.2f));
    p.push_back (std::make_unique<Float>  (pid ("osc1_level"),  "Osc1 Level", Range (0.f, 1.f, 0.001f), 0.8f));
    p.push_back (std::make_unique<Choice> (pid ("osc2_wave"),   "Osc2 Wave", waves, 2));
    p.push_back (std::make_unique<Float>  (pid ("osc2_ratio"),  "Osc2 Ratio", Range (0.25f, 8.f, 0.01f, 0.5f), 1.f));
    p.push_back (std::make_unique<Float>  (pid ("osc2_level"),  "Osc2 Level", Range (0.f, 1.f, 0.001f), 0.f));
    p.push_back (std::make_unique<Float>  (pid ("sub_level"),   "Sub", Range (0.f, 1.f, 0.001f), 0.5f));
    p.push_back (std::make_unique<Float>  (pid ("noise_level"), "Noise", Range (0.f, 1.f, 0.001f), 0.f));

    // FM / Fold / Drive / Filter
    p.push_back (std::make_unique<Float>  (pid ("fm_amount"),   "FM", Range (0.f, 1.f, 0.001f), 0.f));
    p.push_back (std::make_unique<Float>  (pid ("fold"),        "Fold", Range (0.f, 1.f, 0.001f), 0.f));
    p.push_back (std::make_unique<Float>  (pid ("drive"),       "Drive", Range (0.f, 1.f, 0.001f), 0.f));
    p.push_back (std::make_unique<Choice> (pid ("filter_type"), "Filter", juce::StringArray { "LP", "BP", "HP", "Notch" }, 0));
    p.push_back (std::make_unique<Bool>   (pid ("filter_24"),   "24 dB", true));
    p.push_back (std::make_unique<Float>  (pid ("cutoff"),      "Cutoff", Range (20.f, 20000.f, 0.1f, 0.3f), 1200.f));
    p.push_back (std::make_unique<Float>  (pid ("res"),         "Reso", Range (0.f, 1.f, 0.001f), 0.3f));

    // LFO
    p.push_back (std::make_unique<Bool>   (pid ("lfo_sync"),    "Sync", true));
    p.push_back (std::make_unique<Choice> (pid ("lfo_div"),     "Rate", juce::StringArray { "1/1", "1/2", "1/4", "1/4T", "1/8", "1/8T", "1/16", "1/16T", "1/32" }, 2));
    p.push_back (std::make_unique<Float>  (pid ("lfo_hz"),      "Rate Hz", Range (0.1f, 30.f, 0.01f, 0.4f), 3.f));
    p.push_back (std::make_unique<Choice> (pid ("lfo_shape"),   "Shape", juce::StringArray { "Sine", "Tri", "Saw Up", "Saw Down", "Square" }, 0));
    p.push_back (std::make_unique<Bool>   (pid ("lfo_retrig"),  "Retrig", true));
    p.push_back (std::make_unique<Float>  (pid ("lfo_cutoff"),  "> Cutoff", Range (0.f, 1.f, 0.001f), 0.5f));
    p.push_back (std::make_unique<Float>  (pid ("lfo_fm"),      "> FM", Range (0.f, 1.f, 0.001f), 0.f));
    p.push_back (std::make_unique<Float>  (pid ("lfo_fold"),    "> Fold", Range (0.f, 1.f, 0.001f), 0.f));
    p.push_back (std::make_unique<Float>  (pid ("lfo_pitch"),   "> Pitch", Range (0.f, 1.f, 0.001f), 0.f));

    // Pitch / Amp
    p.push_back (std::make_unique<Float>  (pid ("pitch_env"),   "Pitch Env", Range (-24.f, 24.f, 0.1f), 0.f));
    p.push_back (std::make_unique<Float>  (pid ("pitch_decay"), "P.Decay", Range (5.f, 1000.f, 1.f, 0.4f), 80.f));
    p.push_back (std::make_unique<Float>  (pid ("glide"),       "Glide", Range (0.f, 500.f, 1.f, 0.5f), 0.f));
    p.push_back (std::make_unique<Int>    (pid ("bend_range"),  "Bend", 1, 24, 2));
    p.push_back (std::make_unique<Float>  (pid ("amp_a"),       "Attack", Range (0.5f, 2000.f, 0.1f, 0.35f), 1.f));
    p.push_back (std::make_unique<Float>  (pid ("amp_d"),       "Decay", Range (1.f, 2000.f, 1.f, 0.4f), 200.f));
    p.push_back (std::make_unique<Float>  (pid ("amp_s"),       "Sustain", Range (0.f, 1.f, 0.001f), 1.f));
    p.push_back (std::make_unique<Float>  (pid ("amp_r"),       "Release", Range (5.f, 3000.f, 1.f, 0.4f), 80.f));
    p.push_back (std::make_unique<Float>  (pid ("master"),      "Master", Range (0.f, 1.f, 0.001f), 0.7f));

    return { p.begin(), p.end() };
}

RiddimProcessor::RiddimProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

void RiddimProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    synth.prepare (sampleRate);
}

bool RiddimProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

riddim::Params RiddimProcessor::readParams() const
{
    auto f = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };
    riddim::Params p;
    p.osc1Wave = (int) f ("osc1_wave");   p.unison = (int) f ("osc1_unison");
    p.osc1Detune = f ("osc1_detune");     p.osc1Level = f ("osc1_level");
    p.osc2Wave = (int) f ("osc2_wave");   p.osc2Ratio = f ("osc2_ratio");
    p.osc2Level = f ("osc2_level");       p.subLevel = f ("sub_level");   p.noiseLevel = f ("noise_level");
    p.fm = f ("fm_amount");               p.fold = f ("fold");            p.drive = f ("drive");
    p.filterType = (int) f ("filter_type"); p.filter24 = f ("filter_24") > 0.5f;
    p.cutoff = f ("cutoff");              p.res = f ("res");
    p.lfoSync = f ("lfo_sync") > 0.5f;    p.lfoDiv = (int) f ("lfo_div"); p.lfoHz = f ("lfo_hz");
    p.lfoShape = (int) f ("lfo_shape");   p.lfoRetrig = f ("lfo_retrig") > 0.5f;
    p.lfoCutoff = f ("lfo_cutoff");       p.lfoFm = f ("lfo_fm");
    p.lfoFold = f ("lfo_fold");           p.lfoPitch = f ("lfo_pitch");
    p.pitchEnv = f ("pitch_env");         p.pitchDecayMs = f ("pitch_decay");
    p.glideMs = f ("glide");              p.bendRange = (int) f ("bend_range");
    p.ampA = f ("amp_a"); p.ampD = f ("amp_d"); p.ampS = f ("amp_s"); p.ampR = f ("amp_r");
    p.master = f ("master");
    return p;
}

void RiddimProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const auto p = readParams();
    double bpm = 140.0, ppq = -1.0;
    bool playing = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm())         bpm = *b;
            if (auto q = pos->getPpqPosition()) ppq = *q;
            playing = pos->getIsPlaying();
        }

    const int n = buffer.getNumSamples();
    float* L = buffer.getWritePointer (0);
    int cursor = 0;
    auto ppqAt = [&] (int s) { return ppq >= 0.0 ? ppq + s * bpm / 60.0 / currentSampleRate : -1.0; };

    for (const auto meta : midi)
    {
        const int t = std::min (meta.samplePosition, n);
        if (t > cursor)
        {
            synth.render (L + cursor, t - cursor, p, bpm, ppqAt (cursor), playing);
            cursor = t;
        }
        const auto m = meta.getMessage();
        if (m.isNoteOn())                          synth.noteOn (m.getNoteNumber(), m.getFloatVelocity(), p);
        else if (m.isNoteOff())                    synth.noteOff (m.getNoteNumber());
        else if (m.isAllNotesOff() || m.isAllSoundOff()) synth.allOff();
        else if (m.isPitchWheel())                 synth.setPitchBend ((m.getPitchWheelValue() - 8192) / 8192.f);
    }
    if (cursor < n)
        synth.render (L + cursor, n - cursor, p, bpm, ppqAt (cursor), playing);

    if (buffer.getNumChannels() > 1)
        buffer.copyFrom (1, 0, buffer, 0, 0, n);
}

void RiddimProcessor::applyPreset (int index)
{
    const auto& presets = getFactoryPresets();
    if (index < 0 || index >= (int) presets.size()) return;
    currentProgram = index;

    for (auto* prm : getParameters())
        if (auto* r = dynamic_cast<juce::RangedAudioParameter*> (prm))
            r->setValueNotifyingHost (r->getDefaultValue());

    for (const auto& kv : presets[(size_t) index].values)
        if (auto* prm = apvts.getParameter (kv.first))
            prm->setValueNotifyingHost (prm->convertTo0to1 (kv.second));
}

void RiddimProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void RiddimProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            currentProgram = xml->getIntAttribute ("program", 0);
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
        }
}

juce::AudioProcessorEditor* RiddimProcessor::createEditor() { return new RiddimEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RiddimProcessor(); }
