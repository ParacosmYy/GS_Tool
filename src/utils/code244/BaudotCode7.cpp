/**
 * @file BaudotCode7.cpp
 * @brief BaudotCode7 实现
 *
 * 实现Baudot码：扩展FIGS转移字符与奇偶校验位无歧义模式检测。
 */

#include "utils/code244/BaudotCode7.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- ITA2 character tables ---- */
const char BaudotCode7::s_letters[32] = {
    '\0', 'E', '\n', 'A', ' ', 'S', 'I', 'U',
    '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
    'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q',
    'O', 'B', 'G', ' ', 'M', 'X', 'V', '\0'  // 0x1F = LTRS shift
};

const char BaudotCode7::s_figures[32] = {
    '\0', '3', '\n', '-', ' ', '\x07', '8', '7',
    '\r', '2', '4', '\a', ',', '!', ':', '(',
    '5', '"', ')', '2', '#', '6', '0', '1',
    '9', '?', '&', ' ', '.', '/', '=', '\0'  // 0x1F = LTRS shift
};

/* ---- Construction / Destruction ---- */

BaudotCode7::BaudotCode7(QObject *parent) : QObject(parent) {}
BaudotCode7::~BaudotCode7() = default;

/* ---- Configuration ---- */

void BaudotCode7::setParityCheck(bool enabled) { m_parityCheck = enabled; }
void BaudotCode7::setEvenParity(bool even) { m_evenParity = even; }

/* ---- Parity helpers ---- */

bool BaudotCode7::validateParity(quint8 code) const
{
    // Count 1-bits in the 6-bit code (5 data + 1 parity)
    int bits = 0;
    quint8 tmp = code;
    for (int i = 0; i < 6; ++i) {
        bits += (tmp & 1);
        tmp >>= 1;
    }
    return m_evenParity ? (bits % 2 == 0) : (bits % 2 == 1);
}

quint8 BaudotCode7::addParity(quint8 code5) const
{
    int bits = 0;
    quint8 tmp = code5;
    for (int i = 0; i < 5; ++i) { bits += (tmp & 1); tmp >>= 1; }
    // Add parity as bit 5
    bool needSet = m_evenParity ? (bits % 2 != 0) : (bits % 2 == 0);
    return needSet ? (code5 | 0x20) : code5;
}

quint8 BaudotCode7::stripParity(quint8 code6) const
{
    return code6 & 0x1F;
}

/* ---- Reverse lookup ---- */

quint8 BaudotCode7::charToCode(QChar ch, Mode mode) const
{
    char c = ch.toUpper().toLatin1();
    const char* table = (mode == LettersMode) ? s_letters : s_figures;
    for (int i = 0; i < 32; ++i) {
        if (table[i] == c) return static_cast<quint8>(i);
    }
    return 0x04;  // Space as fallback
}

/* ---- Encode ---- */

QVector<quint8> BaudotCode7::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint8> result;
    m_mode = LettersMode;
    int modeSwitches = 0;

    for (int i = 0; i < text.length(); ++i) {
        QChar ch = text[i];
        char c = ch.toUpper().toLatin1();

        // Check if character is in LETTERS table
        bool inLetters = false;
        bool inFigures = false;
        for (int j = 0; j < 32; ++j) {
            if (s_letters[j] == c) inLetters = true;
            if (s_figures[j] == c) inFigures = true;
        }

        // Determine required mode
        Mode neededMode = m_mode;
        if (inFigures && !inLetters) neededMode = FiguresMode;
        else if (inLetters && !inFigures) neededMode = LettersMode;
        else neededMode = LettersMode;  // Default to letters for ambiguous

        // Emit shift if needed
        if (neededMode != m_mode) {
            quint8 shiftCode = (neededMode == FiguresMode) ? FIGS_SHIFT : LTRS_SHIFT;
            if (m_parityCheck) shiftCode = addParity(shiftCode);
            result.append(shiftCode);
            m_mode = neededMode;
            modeSwitches++;
        }

        quint8 code = charToCode(ch, m_mode);
        if (m_parityCheck) code = addParity(code);
        result.append(code);
    }

    m_stats.charsEncoded += text.length();
    m_stats.modeSwitches += modeSwitches;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encodingCompleted(text.length(), modeSwitches, timer.elapsed());
    return result;
}

/* ---- Decode ---- */

QString BaudotCode7::decode(const QVector<quint8>& codes)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    m_mode = LettersMode;
    int parityErrors = 0;

    for (int i = 0; i < codes.size(); ++i) {
        quint8 raw = codes[i];

        // Validate parity
        if (m_parityCheck && !validateParity(raw)) {
            parityErrors++;
            emit parityErrorDetected(i, raw);
        }

        quint8 code = stripParity(raw);

        // Handle shift codes
        if (code == FIGS_SHIFT) { m_mode = FiguresMode; continue; }
        if (code == LTRS_SHIFT) { m_mode = LettersMode; continue; }

        // Decode character
        const char* table = (m_mode == LettersMode) ? s_letters : s_figures;
        char ch = table[code];
        if (ch != '\0') result.append(QChar(ch));
    }

    m_stats.charsDecoded += codes.size();
    m_stats.parityErrors += parityErrors;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Mode detection via parity analysis ---- */

BaudotCode7::Mode BaudotCode7::detectMode(const QVector<quint8>& codes) const
{
    if (!m_parityCheck) return LettersMode;
    return analyzeParity(codes);
}

BaudotCode7::Mode BaudotCode7::analyzeParity(const QVector<quint8>& codes) const
{
    // Count valid parity in both modes to determine current mode
    int lettersValid = 0, figuresValid = 0;
    for (int i = 0; i < codes.size(); ++i) {
        quint8 code = codes[i] & 0x1F;
        // Skip shift codes for analysis
        if (code == FIGS_SHIFT || code == LTRS_SHIFT) continue;
        // Check if character is printable in each table
        if (s_letters[code] != '\0') lettersValid++;
        if (s_figures[code] != '\0') figuresValid++;
    }
    return (figuresValid > lettersValid) ? FiguresMode : LettersMode;
}

/* ---- Accessors ---- */

BaudotCode7::Mode BaudotCode7::currentMode() const { return m_mode; }

/* ---- Reset ---- */

void BaudotCode7::resetStatistics()
{
    m_mode = LettersMode;
    m_stats = Stats{}; m_timeSum = 0.0;
}
