# RiddimForge – synth VST3 para Riddim Dubstep

Synth monofónico (bajo/lead) pensado para riddim moderno: wobble sincronizado al tempo,
FM tipo growl, wavefolder, filtro 24 dB y envolvente de pitch para drops y screeches.

## Cómo compilarlo (Windows / Mac)
**Opción fácil (sin instalar nada): GitHub Actions**
1. Creá un repo en GitHub y subí esta carpeta completa.
2. Pestaña *Actions* → *Build RiddimForge* → esperá ~10 min.
3. Bajá el artefacto `RiddimForge-windows-latest` o `RiddimForge-macos-latest`.

**Opción local:** instalá CMake + Visual Studio (Windows) o Xcode (Mac) y corré:
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release

## Instalar en Ableton
- Windows: copiá `RiddimForge.vst3` a `C:\Program Files\Common Files\VST3\`
- Mac: `~/Library/Audio/Plug-Ins/VST3/` (el .component va en `.../Components/`)
- Ableton → Preferencias → Plug-Ins → activar *VST3 System Folders* y *Rescan*.

## Controles
- **OSC**: Osc1 con unison (hasta 7) + detune, Osc2 (modulador FM / audible), Sub (seno -1 octava), Noise.
- **FM / Fold / Drive**: FM por modulación de fase, wavefolder y saturación tanh.
- **Filtro**: LP/BP/HP/Notch, 12 o 24 dB. *Cutoff = punto cerrado*; el LFO lo abre hasta 6 octavas.
- **LFO**: sync al tempo (1/1 … 1/32, con tresillos) o libre en Hz, 5 formas. *Retrig* reinicia con cada nota;
  apagado, se traba al compás del host. Destinos: Cutoff, FM, Fold, Pitch.
- **Pitch Env / Glide / Bend**: para drops, screeches y dives.
- **Amp ADSR + Master**.

## Presets (menú arriba a la derecha)
Init · Riddim Wub 140 · Gollum Growl · Tripler Chop · Dojo Square Gate · Screech Lead ·
Sub Drop · Metal Fold Riddim · Vowel Yoi · Dirty Ghost · Tearout Stab

Tip: para riddim usá 140 BPM, notas cortas en half-time, y sumá Sub Drop en otra pista.
Los presets están en `Source/Presets.h`: copiá un bloque y cambiá valores para crear los tuyos.
