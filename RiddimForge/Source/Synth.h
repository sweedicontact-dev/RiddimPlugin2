#pragma once
#include <cmath>
#include <algorithm>
#include <vector>
#include <juce_audio_basics/juce_audio_basics.h>

namespace riddim
{
constexpr float kPi    = 3.14159265358979f;
constexpr float kTwoPi = 6.28318530717959f;
constexpr int   kMaxUnison = 7;

struct Params
{
    int   osc1Wave = 0, unison = 1, osc2Wave = 2, filterType = 0, lfoDiv = 2, lfoShape = 0, bendRange = 2;
    bool  filter24 = true, lfoSync = true, lfoRetrig = true;
    float osc1Detune = 0.2f, osc1Level = 0.8f, osc2Ratio = 1.f, osc2Level = 0.f, subLevel = 0.5f, noiseLevel = 0.f;
    float fm = 0.f, fold = 0.f, drive = 0.f, cutoff = 1200.f, res = 0.3f;
    float lfoHz = 3.f, lfoCutoff = 0.5f, lfoFm = 0.f, lfoFold = 0.f, lfoPitch = 0.f;
    float pitchEnv = 0.f, pitchDecayMs = 80.f;
    float ampA = 1.f, ampD = 200.f, ampS = 1.f, ampR = 80.f, glideMs = 0.f, master = 0.7f;
};

inline float wrap01 (float p) { return p - std::floor (p); }

inline float polyBlep (float t, float dt)
{
    if (t < dt)         { t /= dt;               return t + t - t * t - 1.f; }
    if (t > 1.f - dt)   { t = (t - 1.f) / dt;    return t * t + t + t + 1.f; }
    return 0.f;
}

inline float oscSample (int wave, float phase, float dt)
{
    switch (wave)
    {
        case 0:  return 2.f * phase - 1.f - polyBlep (phase, dt);                       // saw
        case 1:  { float v = phase < 0.5f ? 1.f : -1.f;                                  // square
                   v += polyBlep (phase, dt);
                   v -= polyBlep (wrap01 (phase + 0.5f), dt);
                   return v; }
        case 2:  return std::sin (kTwoPi * phase);                                       // sine
        default: return 4.f * std::abs (phase - 0.5f) - 1.f;                             // triangle
    }
}

inline float lfoWave (int shape, float p)
{
    switch (shape)
    {
        case 0:  return -std::cos (kTwoPi * p);          // sine (arranca cerrado)
        case 1:  return 1.f - 4.f * std::abs (p - 0.5f); // triangle
        case 2:  return 2.f * p - 1.f;                   // saw up
        case 3:  return 1.f - 2.f * p;                   // saw down
        default: return p < 0.5f ? 1.f : -1.f;           // square
    }
}

struct SVF
{
    float ic1 = 0.f, ic2 = 0.f;
    // devuelve {low, band, high, notch}
    inline float process (float v0, float g, float k, int type)
    {
        const float a1 = 1.f / (1.f + g * (g + k));
        const float a2 = g * a1;
        const float a3 = g * a2;
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.f * v1 - ic1;
        ic2 = 2.f * v2 - ic2;
        switch (type)
        {
            case 0:  return v2;                    // LP
            case 1:  return v1 * k * 1.5f;         // BP
            case 2:  return v0 - k * v1 - v2;      // HP
            default: return v0 - k * v1;           // Notch
        }
    }
    void reset() { ic1 = ic2 = 0.f; }
};

class Synth
{
public:
    void prepare (double sr)
    {
        sampleRate = (float) sr;
        adsr.setSampleRate (sr);
        allOff();
    }

    void allOff()
    {
        notes.clear();
        adsr.reset();
        f1.reset(); f2.reset();
    }

    void setPitchBend (float semitonesNormalised) { bend = semitonesNormalised; }

    void noteOn (int note, float vel, const Params& p)
    {
        notes.erase (std::remove (notes.begin(), notes.end(), note), notes.end());
        const bool wasIdle = notes.empty() && ! adsr.isActive();
        notes.push_back (note);
        target = (float) note;
        if (wasIdle || p.glideMs <= 0.5f) current = target;
        velocity = 0.4f + 0.6f * vel;
        pitchEnvVal = 1.f;
        for (auto& ph : uPhase) ph = 0.f;
        phase2 = subPhase = 0.f;
        if (p.lfoRetrig) lfoPhase = 0.f;
        adsr.noteOn();
    }

    void noteOff (int note)
    {
        notes.erase (std::remove (notes.begin(), notes.end(), note), notes.end());
        if (notes.empty()) adsr.noteOff();
        else target = (float) notes.back();
    }

    void render (float* out, int n, const Params& p, double bpm, double ppq, bool hostPlaying)
    {
        adsr.setParameters ({ p.ampA * 0.001f, p.ampD * 0.001f, p.ampS, p.ampR * 0.001f });

        const float glideCoef = p.glideMs <= 0.5f ? 1.f : 1.f - std::exp (-1.f / (p.glideMs * 0.001f * sampleRate));
        const float penvCoef  = std::exp (-1.f / (std::max (p.pitchDecayMs, 1.f) * 0.001f * sampleRate));
        const float lfoSmooth = 1.f - std::exp (-1.f / (0.0008f * sampleRate));

        static const float beats[] = { 4.f, 2.f, 1.f, 2.f / 3.f, 0.5f, 1.f / 3.f, 0.25f, 1.f / 6.f, 0.125f };
        const float beatLen = beats[std::clamp (p.lfoDiv, 0, 8)];
        const float lfoFreq = p.lfoSync ? (float) (bpm / 60.0) / beatLen : p.lfoHz;
        const bool  lockToHost = p.lfoSync && ! p.lfoRetrig && hostPlaying && ppq >= 0.0;
        const double ppqPerSample = bpm / 60.0 / sampleRate;

        const float maxCut = std::min (20000.f, sampleRate * 0.45f);
        const float k = 2.05f * (1.f - p.res) + 0.05f;

        for (int i = 0; i < n; ++i)
        {
            if (! adsr.isActive()) { out[i] = 0.f; f1.reset(); f2.reset(); advanceLfo (lfoFreq, lockToHost, ppq, ppqPerSample, beatLen, i); continue; }

            const float env = adsr.getNextSample();

            current += (target - current) * glideCoef;
            pitchEnvVal *= penvCoef;

            // LFO
            advanceLfo (lfoFreq, lockToHost, ppq, ppqPerSample, beatLen, i);
            lfoSm += (lfoWave (p.lfoShape, lfoPhase) - lfoSm) * lfoSmooth;
            const float uni = 0.5f + 0.5f * lfoSm;

            // Pitch
            const float semis = current + p.pitchEnv * pitchEnvVal + p.lfoPitch * 24.f * lfoSm + bend * (float) p.bendRange;
            const float freq  = 440.f * std::pow (2.f, (semis - 69.f) / 12.f);
            const float dt    = std::min (freq / sampleRate, 0.45f);

            // Osc2 (modulador FM + audible)
            const float dt2 = std::min (dt * p.osc2Ratio, 0.45f);
            phase2 = wrap01 (phase2 + dt2);
            const float o2 = oscSample (p.osc2Wave, phase2, dt2);

            const float fmEff = std::clamp (p.fm + p.lfoFm * uni, 0.f, 1.f);
            const float pm = fmEff * 12.f / kTwoPi * o2;          // en ciclos

            // Osc1 con unison
            const int nU = std::clamp (p.unison, 1, kMaxUnison);
            float sum = 0.f;
            for (int u = 0; u < nU; ++u)
            {
                const float spread = nU > 1 ? ((float) u / (float) (nU - 1)) * 2.f - 1.f : 0.f;
                const float f = dt * std::pow (2.f, spread * p.osc1Detune * 50.f / 1200.f);
                uPhase[u] = wrap01 (uPhase[u] + f);
                sum += oscSample (p.osc1Wave, wrap01 (uPhase[u] + pm + 1.f), f);
            }
            sum /= std::sqrt ((float) nU);

            subPhase = wrap01 (subPhase + dt * 0.5f);
            const float sub = std::sin (kTwoPi * subPhase);
            const float noise = rng.nextFloat() * 2.f - 1.f;

            float x = sum * p.osc1Level + o2 * p.osc2Level + sub * p.subLevel + noise * p.noiseLevel;

            // Wavefolder
            const float foldEff = std::clamp (p.fold + p.lfoFold * uni, 0.f, 1.f);
            if (foldEff > 0.001f)
            {
                const float y = std::sin (x * (1.f + foldEff * 8.f) * kPi * 0.5f);
                x += (y - x) * std::min (1.f, foldEff * 5.f);
            }

            // Filtro
            const float cut = std::min (p.cutoff * std::pow (2.f, p.lfoCutoff * 6.f * uni), maxCut);
            const float g = std::tan (kPi * cut / sampleRate);
            float y = f1.process (x, g, k, p.filterType);
            if (p.filter24) y = f2.process (y, g, k, p.filterType);

            // Drive
            if (p.drive > 0.001f)
                y = std::tanh (y * (1.f + p.drive * 24.f)) * 0.9f;

            out[i] = std::tanh (y * env * velocity * p.master * 1.2f);
        }
    }

private:
    void advanceLfo (float freq, bool lock, double ppq, double ppqPerSample, float beatLen, int i)
    {
        if (lock) lfoPhase = (float) wrap01 ((float) ((ppq + ppqPerSample * i) / beatLen));
        else      lfoPhase = wrap01 (lfoPhase + freq / sampleRate);
    }

    float sampleRate = 44100.f;
    std::vector<int> notes;
    float target = 60.f, current = 60.f, velocity = 1.f, pitchEnvVal = 0.f, bend = 0.f;
    float uPhase[kMaxUnison] = {};
    float phase2 = 0.f, subPhase = 0.f, lfoPhase = 0.f, lfoSm = -1.f;
    SVF f1, f2;
    juce::ADSR adsr;
    juce::Random rng;
};
} // namespace riddim
