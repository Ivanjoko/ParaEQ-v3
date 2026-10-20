#pragma once
#include "PluginProcessor.h"

//==============================================================================
//  Estilo cyberpunk: colores neón, esquinas cortadas, brillos
//==============================================================================
namespace Cyber
{
    inline const juce::Colour bg      { 0xff07050f };
    inline const juce::Colour panel   { 0xff110a1f };
    inline const juce::Colour dim     { 0xff2a1f44 };
    inline const juce::Colour cyan    { 0xff00f0ff };
    inline const juce::Colour magenta { 0xffff2bd6 };
    inline const juce::Colour yellow  { 0xfffcee0a };
    inline const juce::Colour green   { 0xff39ff14 };
    inline const juce::Colour orange  { 0xffff7a00 };
    inline const juce::Colour violet  { 0xffb026ff };

    inline juce::Colour band (int b)
    {
        static const juce::Colour c[6] = { cyan, magenta, yellow, green, orange, violet };
        return c[((b % 6) + 6) % 6];
    }

    // Rectángulo con las esquinas superior izquierda e inferior derecha cortadas
    inline juce::Path chamfer (juce::Rectangle<float> r, float c)
    {
        juce::Path p;
        p.startNewSubPath (r.getX() + c, r.getY());
        p.lineTo (r.getRight(), r.getY());
        p.lineTo (r.getRight(), r.getBottom() - c);
        p.lineTo (r.getRight() - c, r.getBottom());
        p.lineTo (r.getX(), r.getBottom());
        p.lineTo (r.getX(), r.getY() + c);
        p.closeSubPath();
        return p;
    }

    inline juce::FontOptions mono (float h, bool bold = false)
    {
        return juce::FontOptions ("Avenir Next", h, bold ? juce::Font::bold : juce::Font::plain);   // letra moderna y limpia
    }

    inline juce::String u8 (const char* s) { return juce::String::fromUTF8 (s); }
}

class CyberLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CyberLookAndFeel();

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float sliderPos, float minPos, float maxPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;
};

//==============================================================================
//  Vúmetro estéreo de LEDs (L y R)
//==============================================================================
class StereoMeter : public juce::Component
{
public:
    explicit StereoMeter (juce::String t) : title (std::move (t)) {}

    void setLevels (float rmsL, float rmsR, float pkL, float pkR)
    {
        rms[0] = rmsL; rms[1] = rmsR; pk[0] = pkL; pk[1] = pkR;
        repaint();
    }
    void paint (juce::Graphics&) override;

private:
    juce::String title;
    float rms[2] { -100.f, -100.f }, pk[2] { -100.f, -100.f };
};

//==============================================================================
//  Panel de información (botón INFO)
//==============================================================================
class InfoOverlay : public juce::Component
{
public:
    InfoOverlay();
    void paint (juce::Graphics&) override;
    void resized() override;
    std::function<void()> onClose;

private:
    juce::TextEditor text;
    juce::TextButton closeBtn { "CERRAR" };
};

//==============================================================================
//  Gráfica: doble clic = añadir banda, arrastrar = frecuencia/ganancia (Shift = fino),
//  rueda = Q, clic derecho = menú, doble clic en un punto = ganancia a 0.
//==============================================================================
class EqGraph : public juce::Component, private juce::Timer
{
public:
    EqGraph (ParaEQProcessor&, std::function<void (int)> onSelectBand);

    void setSelected (int b)     { selected = b; repaint(); }
    void setRange (int db)       { rangeDb = (float) db; repaint(); }
    void setSpectrum (bool on)   { showSpectrum = on; repaint(); }
    void setResonances (bool on) { showRes = on; repaint(); }

    int  addBand (float freq, float gainDb, int type, float q);   // devuelve la banda creada o -1
    void deleteBand (int b);
    int  firstUsed() const;
    int  numUsed() const;
    bool bandUsed (int b) const { return b >= 0 && b < EqMath::numBands && isUsed (b); }

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    struct Resonance { float freq, excess; };

    void timerCallback() override;
    void updateSpectrum();
    void findResonances();
    void showMenu (int band);
    void setParamG (const juce::String& id, float realValue);   // con gesto de edición

    juce::Rectangle<float> plot() const;
    float freqToX (float f) const;
    float xToFreq (float x) const;
    float gainToY (float g) const;
    float yToGain (float y) const;
    float getParam (const juce::String& id) const;
    void  setParam (juce::RangedAudioParameter*, float realValue);
    bool  isUsed (int b) const { return getParam (bandId (b, "used")) > 0.5f; }
    juce::Point<float> nodePos (int band) const;
    int hitTest (juce::Point<float>) const;

    ParaEQProcessor& proc;
    std::function<void (int)> onSelect;
    int selected = -1, dragBand = -1;
    float rangeDb = 18.f;
    bool showSpectrum = true, showRes = false, lastFine = false;
    juce::RangedAudioParameter *dragFreq = nullptr, *dragGain = nullptr;
    juce::Point<float> anchorMouse, anchorNode;

    static constexpr int fftSize = ParaEQProcessor::fftSize;
    juce::dsp::FFT fft { ParaEQProcessor::fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) fftSize, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, 2 * fftSize> fftData {};
    std::array<float, fftSize / 2> spec {}, specSlow {};
    std::vector<Resonance> res;
};

//==============================================================================
class ParaEQEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit ParaEQEditor (ParaEQProcessor&);
    ~ParaEQEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void selectBand (int b);
    void updateEnabled();
    void knob (juce::Slider&);
    void label (juce::Label&, const juce::String&);
    void reselect();
    void runProgrammatic (std::function<void()> change);
    void setPresetName (const juce::String&);
    void showPresetMenu();
    void askSavePreset();
    void loadPresetFile (const juce::File&);

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    CyberLookAndFeel laf;
    ParaEQProcessor& proc;
    EqGraph graph;
    StereoMeter meterIn { "IN" }, meterOut { "OUT" };
    InfoOverlay info;

    juce::Label lblIn, lblOut, lblFreq, lblGain, lblQ, lblRange, lblThr, lblAtk, lblRel, lblHpf, lblLpf;
    juce::Slider inSl, outSl, hpfFreq, lpfFreq, bFreq, bGain, bQ, dRange, dThr, dAtk, dRel;
    juce::TextButton bypassBtn, polBtn, analyzerBtn, resBtn, infoBtn, hpfOn, lpfOn, bandOn, dynBtn, addBtn, delBtn;
    juce::ComboBox hpfSlope, lpfSlope, bType, rangeBox;
    juce::ArrowButton undoBtn { "deshacer", 0.5f, Cyber::yellow }, redoBtn { "rehacer", 0.0f, Cyber::yellow };
    juce::TextButton presetBtn, saveBtn, clearBtn;

    std::unique_ptr<SA> aIn, aOut, aHpfFreq, aLpfFreq, aBFreq, aBGain, aBQ, aDRange, aDThr, aDAtk, aDRel;
    std::unique_ptr<BA> aBypass, aPol, aHpfOn, aLpfOn, aBandOn, aDyn;
    std::unique_ptr<CA> aHpfSlope, aLpfSlope, aBType;

    int selected = -1;
    float inR[2] {}, inP[2] {}, outR[2] {}, outP[2] {};
    juce::Rectangle<int> titleArea, filtersPanel, bandPanel, capFilter, capDyn, undoRect, redoRect;
    juce::String currentPreset;
    juce::uint32 lastInteraction = 0;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParaEQEditor)
};
