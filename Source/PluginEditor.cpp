#include "PluginEditor.h"
#include "Presets.h"

using EqMath::numBands;

static juce::String fmtHz (float f)
{
    return f >= 1000.f ? juce::String (f / 1000.f, 2) + " kHz" : juce::String ((int) std::lround (f)) + " Hz";
}

//==============================================================================
//  CyberLookAndFeel
//==============================================================================
CyberLookAndFeel::CyberLookAndFeel()
{
    setColourScheme (juce::LookAndFeel_V4::getMidnightColourScheme());
    setDefaultSansSerifTypefaceName ("Avenir Next");

    setColour (juce::TextButton::buttonOnColourId, Cyber::cyan);
    setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    setColour (juce::ComboBox::textColourId, juce::Colours::white);
    setColour (juce::PopupMenu::backgroundColourId, Cyber::panel);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Cyber::magenta.withAlpha (0.35f));
    setColour (juce::PopupMenu::textColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, juce::Colours::white);
}

void CyberLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                             bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.f);
    auto path = Cyber::chamfer (r, 6.f);
    const bool on = b.getToggleState();
    const auto neon = b.findColour (juce::TextButton::buttonOnColourId);

    g.setColour (on ? neon.withAlpha (down ? 0.45f : 0.30f)
                    : juce::Colour (0xff160d28).brighter (highlighted ? 0.15f : 0.f));
    g.fillPath (path);

    if (on)
    {
        g.setColour (neon.withAlpha (0.18f));
        g.strokePath (path, juce::PathStrokeType (4.f));
    }
    g.setColour (on ? neon : neon.withAlpha (highlighted ? 0.85f : 0.45f));
    g.strokePath (path, juce::PathStrokeType (1.4f));
}

void CyberLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                         float a0, float a1, juce::Slider& s)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (6.f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto c = bounds.getCentre();
    const float aTo = a0 + pos * (a1 - a0);
    const float arcR = radius - 4.f;

    auto neon = s.findColour (juce::Slider::rotarySliderFillColourId);
    if (! s.isEnabled()) neon = neon.withAlpha (0.4f);

    juce::Path track, val;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.f, a0, a1, true);
    val.addCentredArc   (c.x, c.y, arcR, arcR, 0.f, a0, aTo, true);

    g.setColour (Cyber::dim);
    g.strokePath (track, juce::PathStrokeType (4.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (neon.withAlpha (0.20f));
    g.strokePath (val, juce::PathStrokeType (9.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (neon);
    g.strokePath (val, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const auto p1 = c.getPointOnCircumference (arcR - 10.f, aTo);
    const auto p2 = c.getPointOnCircumference (arcR * 0.30f, aTo);
    g.setColour (juce::Colours::white.withAlpha (s.isEnabled() ? 1.f : 0.4f));
    g.drawLine (juce::Line<float> (p2, p1), 2.f);
}

void CyberLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos,
                                         float minPos, float maxPos, juce::Slider::SliderStyle style, juce::Slider& s)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, sliderPos, minPos, maxPos, style, s);
        return;
    }

    const float cy = (float) y + (float) h * 0.5f;
    const auto neon = s.findColour (juce::Slider::trackColourId);

    g.setColour (Cyber::dim);
    g.fillRoundedRectangle ((float) x, cy - 2.f, (float) w, 4.f, 2.f);
    g.setColour (neon.withAlpha (0.25f));
    g.fillRoundedRectangle ((float) x, cy - 5.f, sliderPos - (float) x, 10.f, 4.f);
    g.setColour (neon);
    g.fillRoundedRectangle ((float) x, cy - 2.f, sliderPos - (float) x, 4.f, 2.f);
    g.setColour (juce::Colours::white);
    g.fillRect (juce::Rectangle<float> (6.f, 16.f).withCentre ({ sliderPos, cy }));
}

void CyberLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    auto p = Cyber::chamfer (juce::Rectangle<float> (0.f, 0.f, (float) w, (float) h).reduced (1.f), 5.f);
    g.setColour (juce::Colour (0xff150d26));
    g.fillPath (p);
    g.setColour (Cyber::cyan.withAlpha (0.6f));
    g.strokePath (p, juce::PathStrokeType (1.2f));

    juce::Path arrow;
    const float ax = (float) w - 22.f, ay = (float) h * 0.5f - 3.f;
    arrow.addTriangle (ax, ay, ax + 10.f, ay, ax + 5.f, ay + 6.f);
    g.setColour (Cyber::cyan);
    g.fillPath (arrow);
}

//==============================================================================
//  StereoMeter
//==============================================================================
void StereoMeter::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    auto frame = Cyber::chamfer (b.reduced (1.f), 8.f);
    g.setColour (Cyber::panel);
    g.fillPath (frame);
    g.setColour (Cyber::cyan.withAlpha (0.5f));
    g.strokePath (frame, juce::PathStrokeType (1.2f));

    auto inner = b.reduced (6.f, 8.f);
    g.setColour (Cyber::cyan);
    g.setFont (Cyber::mono (12.f, true));
    g.drawText (title, inner.removeFromTop (16.f).toNearestInt(), juce::Justification::centred);

    g.setColour (juce::Colours::grey);
    g.setFont (Cyber::mono (11.f));
    g.drawText ("L     R", inner.removeFromBottom (16.f).toNearestInt(), juce::Justification::centred);

    constexpr int nSeg = 40;
    const float minDb = -60.f, maxDb = 6.f, step = (maxDb - minDb) / (float) nSeg;
    const float gap = 26.f;
    const float barW = (inner.getWidth() - gap) * 0.5f;
    const float segH = inner.getHeight() / (float) nSeg;

    for (int c = 0; c < 2; ++c)
    {
        const float x = inner.getX() + (float) c * (barW + gap);
        for (int s = 0; s < nSeg; ++s)
        {
            const float segDb = minDb + step * (float) (s + 1);
            const float y = inner.getBottom() - (float) (s + 1) * segH;
            const juce::Colour col = segDb < -18.f ? Cyber::green
                                   : segDb < -6.f  ? Cyber::yellow
                                   : segDb < 0.f   ? Cyber::orange : Cyber::magenta;

            const bool lit = rms[c] >= segDb - step;
            const bool isPeak = pk[c] >= segDb - step && pk[c] < segDb;

            g.setColour (lit ? col : col.withAlpha (0.10f));
            g.fillRect (x, y + 1.f, barW, segH - 1.5f);
            if (isPeak)
            {
                g.setColour (juce::Colours::white);
                g.fillRect (x, y + 1.f, barW, segH - 1.5f);
            }
        }
    }

    g.setFont (Cyber::mono (9.f));
    g.setColour (juce::Colours::white.withAlpha (0.6f));
    for (float d : { 0.f, -6.f, -12.f, -24.f, -48.f })
    {
        const float y = inner.getBottom() - (d - minDb) / (maxDb - minDb) * inner.getHeight();
        g.drawText (juce::String ((int) d), (int) (inner.getCentreX() - 13.f), (int) y - 5, 26, 10, juce::Justification::centred);
    }
}

//==============================================================================
//  InfoOverlay
//==============================================================================
static juce::String infoText()
{
    return juce::String::fromUTF8 (R"TXT(
PARA//EQ · GUÍA RÁPIDA
══════════════════════

LA GRÁFICA
• Doble clic en un espacio vacío: añade una banda nueva justo ahí (hasta 24).
• Arrastra un punto: izquierda y derecha cambian la frecuencia; arriba y abajo, la ganancia.
• Mantén SHIFT mientras arrastras: ajuste 10 veces más fino.
• Rueda del ratón sobre un punto: cambia la Q (lo ancha o estrecha que es la banda).
• Doble clic sobre un punto: pone su ganancia a 0 dB.
• Clic derecho sobre un punto: menú para eliminar la banda, desactivarla o activar su dinámica.
• La línea brillante es la curva total de todo el ecualizador. El fondo magenta es el analizador
  de espectro: lo que está sonando a la salida.

BARRA SUPERIOR
• ANALIZADOR: muestra u oculta el espectro de fondo.
• RESONANCIAS: marca con triángulos las «frecuencias molestas»: picos que sobresalen del resto del
  sonido (zumbidos, silbidos, notas que «pinchan»). Haz clic en un triángulo y se crea una banda
  estrecha que la reduce. Funciona mejor con el audio sonando unos segundos.
• RANGO: cuántos dB se ven en la gráfica (±6 para ajustes muy finos, ±24 para cambios grandes).
• INFO: abre esta ayuda.
• IN / OUT: volumen de entrada (antes del ecualizador) y de salida (después), en dB.
• Ø POL: invierte la polaridad (le da la vuelta a la onda). Útil cuando dos pistas se anulan al sumarlas.
• BYPASS: desactiva todo el efecto para comparar con el sonido original.

DESHACER Y REHACER
• Las dos flechitas de la segunda fila: ◀ deshace el último cambio y ▶ lo rehace.
  Guardan hasta 100 pasos. Se apagan cuando no hay nada que deshacer.
• BORRAR TODO: quita todas las bandas, los filtros y deja entrada y salida a 0 dB.
  Si te arrepientes, pulsa la flecha de deshacer.

PRESETS
• PRESETS: abre una lista de ajustes listos para usar, agrupados por categorías:
  filtros (HPF y LPF), efectos (llamada de teléfono, radio AM, megáfono, lo-fi, bajo el agua),
  voz (voz clara, presencia, de-esser, quitar barro), música (más graves, brillo, mastering suave)
  y corrección (quitar zumbido de 50 o 60 Hz).
• Al elegir uno, sustituye las bandas y los filtros actuales. Puedes deshacerlo con la flecha.
• GUARDAR: guarda tu ajuste actual con el nombre que quieras. Aparecerá en «Mis presets».
  Los archivos .paraeq se guardan en la carpeta de datos de tu usuario; desde el menú puedes abrirla
  o cargar un preset desde cualquier archivo (por ejemplo, uno que te haya pasado un amigo).

VÚMETROS (a los lados de la gráfica)
• IN (izquierda): nivel de entrada, canales L y R.
• OUT (derecha): nivel de salida, canales L y R.
• La barra muestra el nivel medio y la marca blanca, el pico. Si llega al magenta, estás en 0 dB: cuidado con saturar.

PANEL DE BANDA (abajo a la derecha)
Se aplica a la banda seleccionada (la que tiene el aro blanco en la gráfica).
• Tipo: CAMPANA sube o baja una zona; SHELF GRAVES / AGUDOS sube o baja todo lo grave o todo lo agudo
  desde esa frecuencia; NOTCH corta una frecuencia muy concreta (no tiene ganancia).
• ACTIVA: enciende o apaga la banda sin borrarla.
• FRECUENCIA: frecuencia central en Hz. GANANCIA: cuántos dB sube o baja. Q: ancho; con Q alta la banda es muy estrecha.
• + BANDA / ELIMINAR: añade o borra bandas.
• Puedes escribir valores exactos: doble clic en un número y escribe, por ejemplo «2.5k».

ECUALIZACIÓN DINÁMICA (panel DINÁMICA)
Una banda dinámica solo actúa cuando esa frecuencia suena fuerte. Activa el botón DINÁMICA.
• RANGO: cuánto cambia la ganancia al superar el umbral. Negativo = reduce (como un de-esser suave);
  positivo = realza cuando suena fuerte.
• UMBRAL: nivel a partir del cual empieza a actuar. Más bajo = actúa antes.
• ATAQUE: lo rápido que reacciona (ms). RELEASE: lo rápido que vuelve a la normalidad (ms).
• En la gráfica verás un circulito unido al punto: es la ganancia real en ese instante.

FILTROS DE CORTE (panel izquierdo)
• HPF corta los graves por debajo de la frecuencia elegida (quita ruidos y retumbos).
• LPF corta los agudos por encima de la frecuencia elegida (suaviza el siseo).
• La pendiente (12, 24, 36 o 48 dB/oct) indica lo brusco que es el corte.

CONSEJOS
• Para quitar un zumbido: activa RESONANCIAS, deja sonar el audio y haz clic en el triángulo.
• Para recortar solo cuando molesta (voces con «eses»): una banda en agudos con DINÁMICA y RANGO negativo.
• Todos los cambios son suaves: no se oyen clics al mover o activar bandas.
)TXT");
}

InfoOverlay::InfoOverlay()
{
    text.setMultiLine (true, true);
    text.setReadOnly (true);
    text.setCaretVisible (false);
    text.setScrollbarsShown (true);
    text.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0a0614));
    text.setColour (juce::TextEditor::textColourId, juce::Colours::white);
    text.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    text.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    text.setFont (juce::Font (Cyber::mono (14.f)));
    text.setText (infoText());
    addAndMakeVisible (text);

    closeBtn.setColour (juce::TextButton::buttonOnColourId, Cyber::magenta);
    closeBtn.onClick = [this] { if (onClose) onClose(); };
    addAndMakeVisible (closeBtn);
}

void InfoOverlay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.55f));
    auto r = getLocalBounds().toFloat().reduced (1.f);
    auto frame = Cyber::chamfer (r, 16.f);
    g.setColour (juce::Colour (0xff0a0614));
    g.fillPath (frame);
    g.setColour (Cyber::magenta.withAlpha (0.25f));
    g.strokePath (frame, juce::PathStrokeType (6.f));
    g.setColour (Cyber::magenta);
    g.strokePath (frame, juce::PathStrokeType (1.6f));

    g.setColour (Cyber::cyan);
    g.setFont (Cyber::mono (18.f, true));
    g.drawText ("// INFO", getLocalBounds().reduced (24, 12).removeFromTop (30), juce::Justification::centredLeft);
}

void InfoOverlay::resized()
{
    auto r = getLocalBounds().reduced (24, 14);
    auto head = r.removeFromTop (34);
    closeBtn.setBounds (head.removeFromRight (100).reduced (0, 2));
    r.removeFromTop (6);
    text.setBounds (r);
}

//==============================================================================
//  EqGraph
//==============================================================================
EqGraph::EqGraph (ParaEQProcessor& p, std::function<void (int)> cb)
    : proc (p), onSelect (std::move (cb))
{
    spec.fill (-120.f);
    specSlow.fill (-120.f);
    startTimerHz (30);
}

void EqGraph::timerCallback()
{
    if (showSpectrum || showRes)
        updateSpectrum();
    if (showRes)
        findResonances();
    repaint();
}

void EqGraph::updateSpectrum()
{
    const int mask = ParaEQProcessor::scopeSize - 1;
    const int end = proc.scopePos.load (std::memory_order_acquire);

    for (int i = 0; i < fftSize; ++i)
        fftData[(size_t) i] = proc.scope[(size_t) ((end - fftSize + i) & mask)].load (std::memory_order_relaxed);
    std::fill (fftData.begin() + fftSize, fftData.end(), 0.f);

    window.multiplyWithWindowingTable (fftData.data(), (size_t) fftSize);
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    for (int k = 0; k < fftSize / 2; ++k)
    {
        const float db = juce::Decibels::gainToDecibels (fftData[(size_t) k] / ((float) fftSize * 0.25f), -120.f);
        auto& s = spec[(size_t) k];
        s = db > s ? db : s * 0.85f + db * 0.15f;                         // subida rápida, caída suave
        specSlow[(size_t) k] = specSlow[(size_t) k] * 0.97f + db * 0.03f;  // promedio lento (~1 s) para resonancias
    }
}

// Detecta picos estrechos que sobresalen del nivel de su entorno (frecuencias molestas)
void EqGraph::findResonances()
{
    res.clear();
    const double sr = proc.currentSr.load();
    const float binHz = (float) (sr / fftSize);
    const int kMin = juce::jmax (6, (int) (120.f / binHz));
    const int kMax = juce::jmin (fftSize / 2 - 8, (int) (12000.f / binHz));

    std::vector<Resonance> cand;
    for (int k = kMin; k <= kMax; ++k)
    {
        const float v = specSlow[(size_t) k];
        if (v < -75.f || v < specSlow[(size_t) k - 1] || v < specSlow[(size_t) k + 1])
            continue;

        const int w = juce::jmax (5, k / 5);
        float sum = 0.f;
        int cnt = 0;
        for (int j = k - w; j <= k + w; ++j)
            if (std::abs (j - k) > 1) { sum += specSlow[(size_t) j]; ++cnt; }

        const float excess = v - sum / (float) cnt;
        if (excess > 6.f)
            cand.push_back ({ (float) k * binHz, excess });
    }

    std::sort (cand.begin(), cand.end(), [] (const Resonance& a, const Resonance& b) { return a.excess > b.excess; });
    for (auto& c : cand)
    {
        bool near = false;
        for (auto& r : res)
            if (std::abs (std::log2 (c.freq / r.freq)) < 0.33f) { near = true; break; }
        if (! near) res.push_back (c);
        if (res.size() >= 6) break;
    }
}

juce::Rectangle<float> EqGraph::plot() const
{
    return getLocalBounds().toFloat().reduced (12.f, 12.f).withTrimmedLeft (28.f).withTrimmedBottom (14.f);
}

float EqGraph::freqToX (float f) const
{
    auto r = plot();
    return r.getX() + r.getWidth() * std::log (f / 20.f) / std::log (1000.f);
}

float EqGraph::xToFreq (float x) const
{
    auto r = plot();
    const float t = juce::jlimit (0.f, 1.f, (x - r.getX()) / r.getWidth());
    return 20.f * std::pow (1000.f, t);
}

float EqGraph::gainToY (float g) const
{
    auto r = plot();
    return r.getBottom() - (g + rangeDb) / (2.f * rangeDb) * r.getHeight();
}

float EqGraph::yToGain (float y) const
{
    auto r = plot();
    return juce::jlimit (-24.f, 24.f, -rangeDb + (r.getBottom() - y) / r.getHeight() * 2.f * rangeDb);
}

float EqGraph::getParam (const juce::String& id) const
{
    return proc.apvts.getRawParameterValue (id)->load();
}

void EqGraph::setParam (juce::RangedAudioParameter* p, float v)
{
    if (p != nullptr)
        p->setValueNotifyingHost (p->convertTo0to1 (v));
}

void EqGraph::setParamG (const juce::String& id, float v)
{
    if (auto* p = proc.apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (v));
        p->endChangeGesture();
    }
}

int EqGraph::firstUsed() const
{
    for (int b = 0; b < numBands; ++b)
        if (isUsed (b)) return b;
    return -1;
}

int EqGraph::numUsed() const
{
    int n = 0;
    for (int b = 0; b < numBands; ++b)
        if (isUsed (b)) ++n;
    return n;
}

int EqGraph::addBand (float freq, float gainDb, int type, float q)
{
    for (int b = 0; b < numBands; ++b)
    {
        if (isUsed (b))
            continue;

        setParamG (bandId (b, "type"), (float) type);
        setParamG (bandId (b, "freq"), freq);
        setParamG (bandId (b, "gain"), gainDb);
        setParamG (bandId (b, "q"), q);
        setParamG (bandId (b, "dyn"), 0.f);
        setParamG (bandId (b, "on"), 1.f);
        setParamG (bandId (b, "used"), 1.f);

        selected = b;
        onSelect (b);
        return b;
    }
    return -1;   // las 24 bandas están en uso
}

void EqGraph::deleteBand (int b)
{
    setParamG (bandId (b, "used"), 0.f);
    selected = firstUsed();
    onSelect (selected);
}

juce::Point<float> EqGraph::nodePos (int b) const
{
    const int type = (int) std::lround (getParam (bandId (b, "type")));
    const float g = (type == EqMath::Notch) ? 0.f : getParam (bandId (b, "gain"));
    auto pl = plot();
    return { freqToX (getParam (bandId (b, "freq"))),
             juce::jlimit (pl.getY(), pl.getBottom(), gainToY (g)) };
}

int EqGraph::hitTest (juce::Point<float> p) const
{
    int best = -1;
    float bestD = 14.f;
    for (int b = 0; b < numBands; ++b)
    {
        if (! isUsed (b)) continue;
        const float d = nodePos (b).getDistanceFrom (p);
        if (d < bestD) { bestD = d; best = b; }
    }
    return best;
}

void EqGraph::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    auto frame = Cyber::chamfer (area.reduced (1.f), 12.f);
    g.setColour (juce::Colour (0xff0b0716));
    g.fillPath (frame);
    g.setColour (Cyber::cyan.withAlpha (0.55f));
    g.strokePath (frame, juce::PathStrokeType (1.4f));

    auto pl = plot();
    const double sr = proc.currentSr.load();

    // ---- Rejilla
    g.setFont (Cyber::mono (11.f));
    for (int k = -3; k <= 3; ++k)
    {
        const int d = (int) std::lround (k * rangeDb / 3.f);
        const float y = gainToY ((float) d);
        g.setColour (d == 0 ? Cyber::cyan.withAlpha (0.45f) : Cyber::violet.withAlpha (0.16f));
        g.drawHorizontalLine ((int) y, pl.getX(), pl.getRight());
        g.setColour (Cyber::violet.brighter (0.4f));
        g.drawText ((d > 0 ? "+" : "") + juce::String (d), 0, (int) y - 7, 34, 14, juce::Justification::centredRight);
    }

    const float fl[] = { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 };
    const char* ft[] = { "20", "50", "100", "200", "500", "1k", "2k", "5k", "10k", "20k" };
    for (int i = 0; i < 10; ++i)
    {
        const float x = freqToX (fl[i]);
        g.setColour (Cyber::violet.withAlpha (0.16f));
        g.drawVerticalLine ((int) x, pl.getY(), pl.getBottom());
        g.setColour (Cyber::violet.brighter (0.4f));
        g.drawText (ft[i], (int) x - 16, (int) pl.getBottom() + 1, 32, 14, juce::Justification::centred);
    }

    g.saveState();
    g.reduceClipRegion (pl.toNearestInt());

    // ---- Analizador de espectro (salida)
    if (showSpectrum)
    {
        const float binHz = (float) (sr / fftSize);
        juce::Path sp;
        bool started = false;

        for (float x = pl.getX(); x <= pl.getRight(); x += 2.f)
        {
            const float bLo = xToFreq (x - 1.f) / binHz, bHi = xToFreq (x + 1.f) / binHz;
            float db;
            if (bHi - bLo < 1.f)
            {
                const float bc = 0.5f * (bLo + bHi);
                const int k0 = juce::jlimit (0, fftSize / 2 - 2, (int) bc);
                const float fr = bc - (float) k0;
                db = spec[(size_t) k0] * (1.f - fr) + spec[(size_t) k0 + 1] * fr;
            }
            else
            {
                const int k0 = juce::jmax (1, (int) bLo), k1 = juce::jmin (fftSize / 2 - 1, (int) bHi);
                db = -120.f;
                for (int k = k0; k <= k1; ++k) db = juce::jmax (db, spec[(size_t) k]);
            }
            const float y = juce::jmap (juce::jlimit (-90.f, 0.f, db), -90.f, 0.f, pl.getBottom(), pl.getY());
            if (! started) { sp.startNewSubPath (x, y); started = true; }
            else           sp.lineTo (x, y);
        }
        juce::Path outline (sp);
        sp.lineTo (pl.getRight(), pl.getBottom());
        sp.lineTo (pl.getX(), pl.getBottom());
        sp.closeSubPath();
        g.setColour (Cyber::magenta.withAlpha (0.16f));
        g.fillPath (sp);
        g.setColour (Cyber::magenta.withAlpha (0.55f));
        g.strokePath (outline, juce::PathStrokeType (1.2f));
    }

    // ---- Coeficientes actuales (los mismos que usa el motor de audio)
    const bool bypassed = getParam ("bypass") > 0.5f;
    EqMath::Coef hp[EqMath::maxStages], lp[EqMath::maxStages];
    int nh = 0, nl = 0;
    if (getParam ("hpfOn") > 0.5f) nh = EqMath::designCut (true,  sr, getParam ("hpfFreq"), (int) std::lround (getParam ("hpfSlope")), hp);
    if (getParam ("lpfOn") > 0.5f) nl = EqMath::designCut (false, sr, getParam ("lpfFreq"), (int) std::lround (getParam ("lpfSlope")), lp);

    EqMath::Coef bc[numBands];
    bool act[numBands] {};
    for (int b = 0; b < numBands; ++b)
    {
        act[b] = isUsed (b) && getParam (bandId (b, "on")) > 0.5f;
        if (! act[b]) continue;

        const int type = (int) std::lround (getParam (bandId (b, "type")));
        float gain = getParam (bandId (b, "gain"));
        if (type != EqMath::Notch && getParam (bandId (b, "dyn")) > 0.5f)
            gain += proc.dynShown[(size_t) b].load();          // la curva sigue a la dinámica en vivo
        bc[b] = EqMath::designBand (type, sr, getParam (bandId (b, "freq")), gain, getParam (bandId (b, "q")));
    }

    auto dbToY = [this] (double mag)
    {
        return gainToY (juce::jlimit (-rangeDb, rangeDb, juce::Decibels::gainToDecibels ((float) mag)));
    };

    // ---- Relleno de la banda seleccionada
    if (selected >= 0 && act[selected] && ! bypassed)
    {
        juce::Path p;
        const float y0 = gainToY (0.f);
        p.startNewSubPath (pl.getX(), y0);
        for (float x = pl.getX(); x <= pl.getRight(); x += 2.f)
            p.lineTo (x, dbToY (EqMath::magnitude (bc[selected], xToFreq (x), sr)));
        p.lineTo (pl.getRight(), y0);
        p.closeSubPath();
        g.setColour (Cyber::band (selected).withAlpha (0.20f));
        g.fillPath (p);
    }

    // ---- Curva total (con brillo neón)
    juce::Path curve;
    for (float x = pl.getX(); x <= pl.getRight(); x += 2.f)
    {
        const double f = xToFreq (x);
        double mag = 1.0;
        for (int i = 0; i < nh; ++i) mag *= EqMath::magnitude (hp[i], f, sr);
        for (int i = 0; i < nl; ++i) mag *= EqMath::magnitude (lp[i], f, sr);
        for (int b = 0; b < numBands; ++b) if (act[b]) mag *= EqMath::magnitude (bc[b], f, sr);

        const float y = dbToY (mag);
        if (x == pl.getX()) curve.startNewSubPath (x, y); else curve.lineTo (x, y);
    }
    const auto lineCol = bypassed ? juce::Colours::grey : Cyber::cyan;
    g.setColour (lineCol.withAlpha (0.10f));
    g.strokePath (curve, juce::PathStrokeType (10.f));
    g.setColour (lineCol.withAlpha (0.25f));
    g.strokePath (curve, juce::PathStrokeType (5.f));
    g.setColour (lineCol.brighter (0.5f));
    g.strokePath (curve, juce::PathStrokeType (1.8f));

    g.restoreState();

    // ---- Resonancias (frecuencias molestas)
    if (showRes)
    {
        const float dash[2] = { 4.f, 4.f };
        g.setFont (Cyber::mono (11.f, true));
        for (auto& r : res)
        {
            const float x = freqToX (r.freq);
            g.setColour (Cyber::magenta.withAlpha (0.6f));
            g.drawDashedLine (juce::Line<float> (x, pl.getY() + 12.f, x, pl.getBottom()), dash, 2, 1.f);

            juce::Path tri;
            tri.addTriangle (x - 7.f, pl.getY(), x + 7.f, pl.getY(), x, pl.getY() + 12.f);
            g.setColour (Cyber::magenta.withAlpha (0.35f));
            g.fillPath (tri.createPathWithRoundedCorners (0.f));
            g.setColour (Cyber::magenta);
            g.fillPath (tri);

            const float tx = (x > pl.getRight() - 150.f) ? x - 148.f : x + 8.f;
            g.setColour (juce::Colours::white);
            g.drawText (fmtHz (r.freq) + " +" + juce::String ((int) std::lround (r.excess)) + "dB",
                        (int) tx, (int) pl.getY() + 1, 140, 14, juce::Justification::centredLeft);
        }
    }

    // ---- Puntos de las bandas
    for (int b = 0; b < numBands; ++b)
    {
        if (! isUsed (b)) continue;

        auto c = nodePos (b);
        auto r = juce::Rectangle<float> (18.f, 18.f).withCentre (c);
        const auto col = Cyber::band (b);

        if (act[b])
        {
            g.setColour (col.withAlpha (0.25f));
            g.fillEllipse (r.expanded (5.f));
            g.setColour (col);
            g.fillEllipse (r);
        }
        else
        {
            g.setColour (Cyber::bg);
            g.fillEllipse (r);
            g.setColour (col.withAlpha (0.6f));
            g.drawEllipse (r, 2.f);
        }

        const int type = (int) std::lround (getParam (bandId (b, "type")));
        if (type != EqMath::Notch && getParam (bandId (b, "dyn")) > 0.5f)
        {
            // Banda dinámica: aro exterior y marca con la ganancia real en este instante
            g.setColour (juce::Colours::white.withAlpha (0.7f));
            g.drawEllipse (r.expanded (3.f), 1.f);

            const float off = proc.dynShown[(size_t) b].load();
            if (std::abs (off) > 0.1f)
            {
                auto pl2 = plot();
                const float gy = juce::jlimit (pl2.getY(), pl2.getBottom(), gainToY (getParam (bandId (b, "gain")) + off));
                g.setColour (col.withAlpha (0.8f));
                g.drawLine (c.x, c.y, c.x, gy, 1.5f);
                g.drawEllipse (juce::Rectangle<float> (10.f, 10.f).withCentre ({ c.x, gy }), 1.5f);
            }
        }

        g.setColour (act[b] ? juce::Colours::black : col);
        g.setFont (Cyber::mono (11.f, true));
        g.drawText (juce::String (b + 1), r.toNearestInt(), juce::Justification::centred);

        if (b == selected)
        {
            g.setColour (juce::Colours::white);
            g.drawEllipse (r.expanded (5.f), 2.f);
        }
    }

    // ---- Lectura de la banda seleccionada y ayuda
    g.setFont (Cyber::mono (12.f, true));
    if (selected >= 0 && isUsed (selected))
    {
        auto txt = [this] (const juce::String& id) { return proc.apvts.getParameter (id)->getCurrentValueAsText(); };
        g.setColour (Cyber::band (selected));
        g.drawText ("BANDA " + juce::String (selected + 1) + "  //  " + txt (bandId (selected, "freq"))
                    + "  //  " + txt (bandId (selected, "gain")) + "  //  Q " + txt (bandId (selected, "q")),
                    (int) pl.getX() + 8, (int) pl.getBottom() - 20, 420, 16, juce::Justification::centredLeft);
    }
    g.setColour (juce::Colours::white.withAlpha (0.5f));
    g.drawText (juce::String ("BANDAS ") + juce::String (numUsed()) + "/" + juce::String (numBands)
                + Cyber::u8 ("   //   doble clic: añadir banda"),
                (int) pl.getRight() - 330, (int) pl.getBottom() - 20, 322, 16, juce::Justification::centredRight);
}

void EqGraph::showMenu (int b)
{
    juce::PopupMenu m;
    m.addItem (1, "Eliminar banda");
    m.addItem (2, getParam (bandId (b, "on")) > 0.5f ? "Desactivar banda" : "Activar banda");
    m.addItem (3, getParam (bandId (b, "dyn")) > 0.5f ? Cyber::u8 ("Quitar dinámica") : Cyber::u8 ("Activar dinámica"));

    m.showMenuAsync (juce::PopupMenu::Options(), [this, b] (int r)
    {
        if (r == 1)      deleteBand (b);
        else if (r == 2) setParamG (bandId (b, "on"),  getParam (bandId (b, "on"))  > 0.5f ? 0.f : 1.f);
        else if (r == 3) setParamG (bandId (b, "dyn"), getParam (bandId (b, "dyn")) > 0.5f ? 0.f : 1.f);
    });
}

void EqGraph::mouseDown (const juce::MouseEvent& e)
{
    dragBand = -1;

    // Clic en el triángulo de una resonancia: crea una banda que la reduce
    if (showRes)
        for (auto& r : res)
            if (std::abs (e.position.x - freqToX (r.freq)) < 10.f && e.position.y < plot().getY() + 24.f)
            {
                addBand (r.freq, -juce::jlimit (3.f, 12.f, r.excess * 0.7f), EqMath::Bell, 6.f);
                return;
            }

    const int b = hitTest (e.position);
    if (b < 0)
        return;

    selected = b;
    onSelect (b);

    if (e.mods.isPopupMenu())
    {
        showMenu (b);
        return;
    }

    dragBand = b;
    dragFreq = proc.apvts.getParameter (bandId (b, "freq"));
    dragGain = proc.apvts.getParameter (bandId (b, "gain"));
    dragFreq->beginChangeGesture();
    dragGain->beginChangeGesture();

    anchorMouse = e.position;
    anchorNode  = nodePos (b);
    lastFine    = e.mods.isShiftDown();
}

void EqGraph::mouseDrag (const juce::MouseEvent& e)
{
    if (dragBand < 0)
        return;

    // Si la banda estaba apagada, al arrastrarla se enciende
    if (auto* on = proc.apvts.getParameter (bandId (dragBand, "on")))
        if (on->getValue() < 0.5f)
            on->setValueNotifyingHost (1.f);

    // Shift = ajuste fino (10 veces más lento)
    const bool fine = e.mods.isShiftDown();
    if (fine != lastFine)
    {
        anchorMouse = e.position;
        anchorNode  = nodePos (dragBand);
        lastFine    = fine;
    }
    const auto target = anchorNode + (e.position - anchorMouse) * (fine ? 0.1f : 1.0f);

    setParam (dragFreq, xToFreq (target.x));
    if ((int) std::lround (getParam (bandId (dragBand, "type"))) != EqMath::Notch)
        setParam (dragGain, yToGain (target.y));
}

void EqGraph::mouseUp (const juce::MouseEvent&)
{
    if (dragBand < 0)
        return;

    dragFreq->endChangeGesture();
    dragGain->endChangeGesture();
    dragBand = -1;
    dragFreq = dragGain = nullptr;
}

void EqGraph::mouseDoubleClick (const juce::MouseEvent& e)
{
    const int b = hitTest (e.position);
    if (b >= 0)
    {
        setParamG (bandId (b, "gain"), 0.f);
        return;
    }

    // Doble clic en un espacio vacío: nueva banda
    if (plot().contains (e.position))
        addBand (xToFreq (e.position.x), yToGain (e.position.y), EqMath::Bell, 1.f);
}

void EqGraph::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w)
{
    int b = hitTest (e.position);
    if (b < 0) b = selected;
    if (b < 0) return;

    const float q = getParam (bandId (b, "q"));
    setParamG (bandId (b, "q"), q * std::exp (w.deltaY * (e.mods.isShiftDown() ? 0.15f : 1.5f)));
}

//==============================================================================
//  ParaEQEditor
//==============================================================================
ParaEQEditor::ParaEQEditor (ParaEQProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      graph (p, [this] (int b) { selectBand (b); })
{
    setLookAndFeel (&laf);

    addAndMakeVisible (graph);
    addAndMakeVisible (meterIn);
    addAndMakeVisible (meterOut);

    label (lblIn, "IN");          label (lblOut, "OUT");
    label (lblFreq, "FRECUENCIA");      label (lblGain, "GANANCIA");       label (lblQ, "Q");
    label (lblRange, "RANGO");    label (lblThr, "UMBRAL");     label (lblAtk, "ATAQUE");  label (lblRel, "RELEASE");
    label (lblHpf, "HPF");        label (lblLpf, "LPF");

    for (auto* s : { &inSl, &outSl })
    {
        s->setSliderStyle (juce::Slider::LinearHorizontal);
        s->setTextBoxStyle (juce::Slider::TextBoxRight, false, 78, 20);
        s->setColour (juce::Slider::trackColourId, Cyber::cyan);
        addAndMakeVisible (*s);
    }

    auto toggle = [this] (juce::TextButton& b, const juce::String& text, juce::Colour neon)
    {
        b.setButtonText (text);
        b.setClickingTogglesState (true);
        b.setColour (juce::TextButton::buttonOnColourId, neon);
        addAndMakeVisible (b);
    };
    toggle (bypassBtn, "BYPASS", Cyber::orange);
    toggle (polBtn, Cyber::u8 ("Ø POL"), Cyber::magenta);
    toggle (analyzerBtn, "ANALIZADOR", Cyber::cyan);
    toggle (resBtn, "RESONANCIAS", Cyber::magenta);
    toggle (hpfOn, "ON", Cyber::green);
    toggle (lpfOn, "ON", Cyber::green);
    toggle (bandOn, "ACTIVA", Cyber::green);
    toggle (dynBtn, Cyber::u8 ("DINÁMICA"), Cyber::yellow);

    analyzerBtn.setToggleState (true, juce::dontSendNotification);
    analyzerBtn.onClick = [this] { graph.setSpectrum (analyzerBtn.getToggleState()); };
    resBtn.onClick = [this] { graph.setResonances (resBtn.getToggleState()); };

    infoBtn.setButtonText ("INFO");
    infoBtn.setColour (juce::TextButton::buttonOnColourId, Cyber::yellow);
    infoBtn.onClick = [this] { info.setVisible (true); info.toFront (true); };
    addAndMakeVisible (infoBtn);

    addBtn.setButtonText (Cyber::u8 ("+ BANDA"));
    addBtn.setColour (juce::TextButton::buttonOnColourId, Cyber::cyan);
    addBtn.onClick = [this] { graph.addBand (1000.f, 0.f, EqMath::Bell, 1.f); };
    addAndMakeVisible (addBtn);

    delBtn.setButtonText ("ELIMINAR");
    delBtn.setColour (juce::TextButton::buttonOnColourId, Cyber::magenta);
    delBtn.onClick = [this] { if (selected >= 0) graph.deleteBand (selected); };
    addAndMakeVisible (delBtn);

    rangeBox.addItem (Cyber::u8 ("Rango ±6 dB"), 1);
    rangeBox.addItem (Cyber::u8 ("Rango ±12 dB"), 2);
    rangeBox.addItem (Cyber::u8 ("Rango ±18 dB"), 3);
    rangeBox.addItem (Cyber::u8 ("Rango ±24 dB"), 4);
    rangeBox.setSelectedId (3, juce::dontSendNotification);
    rangeBox.onChange = [this] { graph.setRange (rangeBox.getSelectedId() * 6); };
    addAndMakeVisible (rangeBox);

    knob (hpfFreq);
    knob (lpfFreq);
    for (auto* c : { &hpfSlope, &lpfSlope })
    {
        c->addItemList ({ "12 dB/oct", "24 dB/oct", "36 dB/oct", "48 dB/oct" }, 1);
        addAndMakeVisible (*c);
    }

    bType.addItemList ({ "Campana", "Shelf graves", "Shelf agudos", "Notch" }, 1);
    addAndMakeVisible (bType);
    for (auto* s : { &bFreq, &bGain, &bQ, &dRange, &dThr, &dAtk, &dRel })
        knob (*s);

    for (auto* l : { &lblIn, &lblOut, &lblFreq, &lblGain, &lblQ, &lblRange, &lblThr, &lblAtk, &lblRel, &lblHpf, &lblLpf })
        addAndMakeVisible (*l);

    // ---- Deshacer / rehacer (flechitas), presets y borrar todo
    addAndMakeVisible (undoBtn);
    addAndMakeVisible (redoBtn);
    undoBtn.onClick = [this] { proc.undo(); reselect(); };
    redoBtn.onClick = [this] { proc.redo(); reselect(); };

    presetBtn.setButtonText ("PRESETS");
    presetBtn.setColour (juce::TextButton::buttonOnColourId, Cyber::cyan);
    presetBtn.onClick = [this] { showPresetMenu(); };
    addAndMakeVisible (presetBtn);

    saveBtn.setButtonText ("GUARDAR");
    saveBtn.setColour (juce::TextButton::buttonOnColourId, Cyber::green);
    saveBtn.onClick = [this] { askSavePreset(); };
    addAndMakeVisible (saveBtn);

    clearBtn.setButtonText ("BORRAR TODO");
    clearBtn.setColour (juce::TextButton::buttonOnColourId, Cyber::magenta);
    clearBtn.onClick = [this]
    {
        runProgrammatic ([this] { Presets::clearEq (proc.apvts, true); });
        setPresetName ({});
    };
    addAndMakeVisible (clearBtn);

    addChildComponent (info);
    info.onClose = [this] { info.setVisible (false); };

    auto& a = proc.apvts;
    aIn = std::make_unique<SA> (a, "inGain", inSl);
    aOut = std::make_unique<SA> (a, "outGain", outSl);
    aBypass = std::make_unique<BA> (a, "bypass", bypassBtn);
    aPol = std::make_unique<BA> (a, "polarity", polBtn);
    aHpfOn = std::make_unique<BA> (a, "hpfOn", hpfOn);
    aHpfFreq = std::make_unique<SA> (a, "hpfFreq", hpfFreq);
    aHpfSlope = std::make_unique<CA> (a, "hpfSlope", hpfSlope);
    aLpfOn = std::make_unique<BA> (a, "lpfOn", lpfOn);
    aLpfFreq = std::make_unique<SA> (a, "lpfFreq", lpfFreq);
    aLpfSlope = std::make_unique<CA> (a, "lpfSlope", lpfSlope);

    selectBand (graph.firstUsed());
    setSize (1180, 800);
    startTimerHz (30);
}

ParaEQEditor::~ParaEQEditor()
{
    setLookAndFeel (nullptr);
}

void ParaEQEditor::knob (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 18);
    s.setColour (juce::Slider::rotarySliderFillColourId, Cyber::cyan);
    addAndMakeVisible (s);
}

void ParaEQEditor::label (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setColour (juce::Label::textColourId, Cyber::cyan.withAlpha (0.85f));
    l.setFont (juce::Font (Cyber::mono (11.f, true)));
}

void ParaEQEditor::selectBand (int b)
{
    selected = b;
    graph.setSelected (b);

    aBandOn.reset(); aDyn.reset(); aBType.reset(); aBFreq.reset(); aBGain.reset(); aBQ.reset();
    aDRange.reset(); aDThr.reset(); aDAtk.reset(); aDRel.reset();

    if (b >= 0)
    {
        auto& a = proc.apvts;
        aBandOn = std::make_unique<BA> (a, bandId (b, "on"), bandOn);
        aDyn    = std::make_unique<BA> (a, bandId (b, "dyn"), dynBtn);
        aBType  = std::make_unique<CA> (a, bandId (b, "type"), bType);
        aBFreq  = std::make_unique<SA> (a, bandId (b, "freq"), bFreq);
        aBGain  = std::make_unique<SA> (a, bandId (b, "gain"), bGain);
        aBQ     = std::make_unique<SA> (a, bandId (b, "q"), bQ);
        aDRange = std::make_unique<SA> (a, bandId (b, "dRange"), dRange);
        aDThr   = std::make_unique<SA> (a, bandId (b, "dThr"), dThr);
        aDAtk   = std::make_unique<SA> (a, bandId (b, "dAtk"), dAtk);
        aDRel   = std::make_unique<SA> (a, bandId (b, "dRel"), dRel);

        for (auto* s : { &bFreq, &bGain, &bQ, &dRange, &dThr, &dAtk, &dRel })
            s->setColour (juce::Slider::rotarySliderFillColourId, Cyber::band (b));
    }

    updateEnabled();
    repaint();
}

void ParaEQEditor::updateEnabled()
{
    const bool has = selected >= 0;
    const bool notch = has && (bType.getSelectedId() - 1) == EqMath::Notch;
    const bool dynOn = has && ! notch && dynBtn.getToggleState();

    auto en = [] (juce::Component& c, bool on)
    {
        if (c.isEnabled() != on) c.setEnabled (on);
        c.setAlpha (on ? 1.f : 0.35f);
    };
    en (bandOn, has);  en (bType, has);  en (bFreq, has);  en (bQ, has);  en (delBtn, has);
    en (bGain, has && ! notch);  en (dynBtn, has && ! notch);
    en (dRange, dynOn);  en (dThr, dynOn);  en (dAtk, dynOn);  en (dRel, dynOn);
}

void ParaEQEditor::timerCallback()
{
    auto db = [] (float amp) { return juce::Decibels::gainToDecibels (amp, -100.f); };

    for (size_t c = 0; c < 2; ++c)
    {
        inR[c]  = juce::jmax (std::sqrt (2.f * proc.inMs[c].load()),  inR[c]  * 0.90f);
        outR[c] = juce::jmax (std::sqrt (2.f * proc.outMs[c].load()), outR[c] * 0.90f);
        inP[c]  = juce::jmax (proc.inPk[c].load(),  inP[c]  * 0.97f);
        outP[c] = juce::jmax (proc.outPk[c].load(), outP[c] * 0.97f);
    }
    meterIn.setLevels  (db (inR[0]),  db (inR[1]),  db (inP[0]),  db (inP[1]));
    meterOut.setLevels (db (outR[0]), db (outR[1]), db (outP[0]), db (outP[1]));

    updateEnabled();

    // Historial de deshacer / rehacer: solo se anotan cambios hechos con el ratón o el teclado
    const auto now = juce::Time::getMillisecondCounter();
    const bool rawInteracting = juce::ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown()
                             || dynamic_cast<juce::TextEditor*> (juce::Component::getCurrentlyFocusedComponent()) != nullptr;
    if (rawInteracting) lastInteraction = now;
    proc.historyTick (now - lastInteraction < 800u);

    const bool cu = proc.canUndo(), cr = proc.canRedo();
    undoBtn.setEnabled (cu);  undoBtn.setAlpha (cu ? 1.f : 0.3f);
    redoBtn.setEnabled (cr);  redoBtn.setAlpha (cr ? 1.f : 0.3f);
}

void ParaEQEditor::resized()
{
    auto r = getLocalBounds().reduced (12);

    auto top = r.removeFromTop (50);
    titleArea = top.removeFromLeft (150);
    analyzerBtn.setBounds (top.removeFromLeft (100).reduced (0, 10));  top.removeFromLeft (6);
    resBtn.setBounds      (top.removeFromLeft (112).reduced (0, 10));  top.removeFromLeft (6);
    rangeBox.setBounds    (top.removeFromLeft (112).reduced (0, 10));  top.removeFromLeft (6);
    infoBtn.setBounds     (top.removeFromLeft (56).reduced (0, 10));

    bypassBtn.setBounds (top.removeFromRight (78).reduced (0, 10));  top.removeFromRight (6);
    polBtn.setBounds    (top.removeFromRight (78).reduced (0, 10));  top.removeFromRight (10);

    auto placeGain = [] (juce::Label& l, juce::Slider& s, juce::Rectangle<int> b)
    {
        l.setBounds (b.removeFromLeft (34));
        s.setBounds (b);
    };
    placeGain (lblOut, outSl, top.removeFromRight (190));  top.removeFromRight (8);
    placeGain (lblIn,  inSl,  top.removeFromRight (190));

    // Segunda fila: deshacer / rehacer, presets, borrar todo
    r.removeFromTop (4);
    {
        auto row2 = r.removeFromTop (36);
        undoRect = row2.removeFromLeft (34);  row2.removeFromLeft (6);
        redoRect = row2.removeFromLeft (34);  row2.removeFromLeft (14);
        undoBtn.setBounds (undoRect.reduced (10, 10));
        redoBtn.setBounds (redoRect.reduced (10, 10));
        presetBtn.setBounds (row2.removeFromLeft (300).reduced (0, 3));  row2.removeFromLeft (6);
        saveBtn.setBounds   (row2.removeFromLeft (100).reduced (0, 3));  row2.removeFromLeft (14);
        clearBtn.setBounds  (row2.removeFromLeft (120).reduced (0, 3));
    }

    r.removeFromTop (6);
    auto mid = r.removeFromTop (400);
    meterIn.setBounds (mid.removeFromLeft (84));   mid.removeFromLeft (8);
    meterOut.setBounds (mid.removeFromRight (84)); mid.removeFromRight (8);
    graph.setBounds (mid);

    r.removeFromTop (10);
    filtersPanel = r.removeFromLeft (290);
    r.removeFromLeft (10);
    bandPanel = r;

    // ---- Filtros de corte
    {
        auto fp = filtersPanel.reduced (12, 10);
        fp.removeFromTop (22);
        const int half = fp.getWidth() / 2;

        auto layoutCut = [] (juce::Rectangle<int> c, juce::Label& l, juce::TextButton& on, juce::Slider& f, juce::ComboBox& sl)
        {
            auto head = c.removeFromTop (26);
            l.setBounds (head.removeFromLeft (44));
            on.setBounds (head.reduced (4, 0));
            sl.setBounds (c.removeFromBottom (26).reduced (4, 0));
            c.removeFromBottom (4);
            f.setBounds (c);
        };
        layoutCut (fp.removeFromLeft (half), lblHpf, hpfOn, hpfFreq, hpfSlope);
        layoutCut (fp, lblLpf, lpfOn, lpfFreq, lpfSlope);
    }

    // ---- Panel de banda
    {
        auto bp = bandPanel.reduced (12, 10);
        bp.removeFromTop (22);

        auto left = bp.removeFromLeft (170);
        bType.setBounds (left.removeFromTop (28));   left.removeFromTop (6);
        bandOn.setBounds (left.removeFromTop (26));  left.removeFromTop (6);
        dynBtn.setBounds (left.removeFromTop (26));  left.removeFromTop (6);
        addBtn.setBounds (left.removeFromTop (26));  left.removeFromTop (6);
        delBtn.setBounds (left.removeFromTop (26));
        bp.removeFromLeft (10);

        const int kw = bp.getWidth() / 7;
        auto caps = bp.removeFromTop (16);
        capFilter = caps.removeFromLeft (kw * 3);
        capDyn = caps;

        auto place = [&] (juce::Label& l, juce::Slider& s)
        {
            auto col = bp.removeFromLeft (kw);
            l.setBounds (col.removeFromTop (16));
            s.setBounds (col);
        };
        place (lblFreq, bFreq);  place (lblGain, bGain);  place (lblQ, bQ);
        place (lblRange, dRange); place (lblThr, dThr);   place (lblAtk, dAtk);  place (lblRel, dRel);
    }

    info.setBounds (getLocalBounds().reduced (60, 40));
}

void ParaEQEditor::paint (juce::Graphics& g)
{
    g.fillAll (Cyber::bg);

    // Líneas de escaneo
    g.setColour (juce::Colours::black.withAlpha (0.18f));
    for (int y = 0; y < getHeight(); y += 3)
        g.drawHorizontalLine (y, 0.f, (float) getWidth());

    // Marcos de las flechitas de deshacer / rehacer
    for (auto rr : { undoRect, redoRect })
    {
        auto p = Cyber::chamfer (rr.toFloat().reduced (1.f), 5.f);
        g.setColour (Cyber::panel);
        g.fillPath (p);
        g.setColour (Cyber::yellow.withAlpha (0.6f));
        g.strokePath (p, juce::PathStrokeType (1.2f));
    }

    // Título con brillo
    g.setFont (Cyber::mono (28.f, true));
    g.setColour (Cyber::magenta.withAlpha (0.35f));
    g.drawText ("PARA//EQ", titleArea.translated (2, 1), juce::Justification::centredLeft);
    g.setColour (Cyber::cyan);
    g.drawText ("PARA//EQ", titleArea, juce::Justification::centredLeft);

    for (auto r : { filtersPanel, bandPanel })
    {
        auto p = Cyber::chamfer (r.toFloat().reduced (1.f), 12.f);
        g.setColour (Cyber::panel);
        g.fillPath (p);
        g.setColour (Cyber::cyan.withAlpha (0.18f));
        g.strokePath (p, juce::PathStrokeType (5.f));
        g.setColour (Cyber::cyan.withAlpha (0.65f));
        g.strokePath (p, juce::PathStrokeType (1.3f));
    }

    g.setFont (Cyber::mono (12.f, true));
    g.setColour (Cyber::magenta);
    g.drawText ("// FILTROS DE CORTE", filtersPanel.reduced (16, 10).removeFromTop (18), juce::Justification::centredLeft);

    juce::String title = "// SIN BANDA SELECCIONADA";
    if (selected >= 0)
        title = "// BANDA " + juce::String (selected + 1) + "  -  " + bType.getText().toUpperCase();
    g.drawText (title, bandPanel.reduced (16, 10).removeFromTop (18), juce::Justification::centredLeft);

    g.setFont (Cyber::mono (11.f, true));
    g.setColour (juce::Colours::white.withAlpha (0.55f));
    g.drawText ("FILTRO", capFilter, juce::Justification::centred);
    g.drawText (Cyber::u8 ("DINÁMICA"), capDyn, juce::Justification::centred);
}


//==============================================================================
//  Deshacer, presets y borrar todo
//==============================================================================
void ParaEQEditor::reselect()
{
    selectBand ((selected >= 0 && graph.bandUsed (selected)) ? selected : graph.firstUsed());
}

// Ejecuta un cambio hecho por código (preset, borrar todo) guardándolo en el historial
void ParaEQEditor::runProgrammatic (std::function<void()> change)
{
    proc.historyCheckpoint();
    change();
    proc.historyAfterProgrammatic();
    selectBand (graph.firstUsed());
}

void ParaEQEditor::setPresetName (const juce::String& n)
{
    currentPreset = n;
    presetBtn.setButtonText (n.isEmpty() ? juce::String ("PRESETS") : "PRESET: " + n);
}

void ParaEQEditor::showPresetMenu()
{
    juce::PopupMenu menu;
    const auto& list = Presets::factory();

    juce::PopupMenu sub;
    juce::String cat;
    for (size_t i = 0; i < list.size(); ++i)
    {
        const auto c = Cyber::u8 (list[i].category);
        if (c != cat)
        {
            if (cat.isNotEmpty()) menu.addSubMenu (cat, sub);
            sub = juce::PopupMenu();
            cat = c;
        }
        sub.addItem (1000 + (int) i, Cyber::u8 (list[i].name));
    }
    if (cat.isNotEmpty()) menu.addSubMenu (cat, sub);

    menu.addSeparator();

    const auto files = Presets::userFiles();
    juce::PopupMenu mine;
    if (files.empty())
        mine.addItem (9, Cyber::u8 ("(vacío)"), false, false);
    for (size_t i = 0; i < files.size(); ++i)
        mine.addItem (2000 + (int) i, files[i].getFileNameWithoutExtension());
    menu.addSubMenu (Cyber::u8 ("Mis presets"), mine);

    menu.addSeparator();
    menu.addItem (1, Cyber::u8 ("Guardar preset actual..."));
    menu.addItem (2, "Cargar desde archivo...");
    menu.addItem (3, "Abrir carpeta de presets");

    juce::Component::SafePointer<ParaEQEditor> self (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&presetBtn), [self, files] (int r)
    {
        if (self == nullptr || r == 0)
            return;

        if (r >= 2000)
        {
            self->loadPresetFile (files[(size_t) (r - 2000)]);
        }
        else if (r >= 1000)
        {
            const auto& p = Presets::factory()[(size_t) (r - 1000)];
            self->runProgrammatic ([self, &p] { Presets::apply (self->proc.apvts, p); });
            self->setPresetName (Cyber::u8 (p.name));
        }
        else if (r == 1)
        {
            self->askSavePreset();
        }
        else if (r == 2)
        {
            self->fileChooser = std::make_unique<juce::FileChooser> ("Elige un preset", Presets::userDir(), "*.paraeq");
            self->fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                            [self] (const juce::FileChooser& fc)
                                            {
                                                const auto f = fc.getResult();
                                                if (self != nullptr && f.existsAsFile())
                                                    self->loadPresetFile (f);
                                            });
        }
        else if (r == 3)
        {
            Presets::userDir().createDirectory();
            Presets::userDir().revealToUser();
        }
    });
}

void ParaEQEditor::loadPresetFile (const juce::File& f)
{
    if (auto xml = juce::parseXML (f))
    {
        runProgrammatic ([this, &xml] { Presets::applyXml (proc.apvts, *xml); });
        setPresetName (f.getFileNameWithoutExtension());
    }
}

void ParaEQEditor::askSavePreset()
{
    auto* w = new juce::AlertWindow (Cyber::u8 ("Guardar preset"), Cyber::u8 ("Escribe un nombre para tu preset:"),
                                     juce::MessageBoxIconType::NoIcon, this);
    w->addTextEditor ("name", currentPreset.isEmpty() ? juce::String ("Mi preset") : currentPreset, juce::String());
    w->addButton ("Guardar", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Cancelar", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    juce::Component::SafePointer<ParaEQEditor> self (this);
    w->enterModalState (true, juce::ModalCallbackFunction::create ([self, w] (int result)
    {
        if (result != 1 || self == nullptr)
            return;

        const auto name = w->getTextEditorContents ("name").trim();
        if (name.isNotEmpty() && Presets::saveUser (self->proc.apvts, name))
            self->setPresetName (name);
    }), true);
}
