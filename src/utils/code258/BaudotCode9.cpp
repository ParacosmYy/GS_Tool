/**
 * @file BaudotCode9.cpp
 * @brief BaudotCode9 实现
 *
 * 实现博多码：仅图型转义编码与5位奇偶校验纠错启发式。
 */

#include "utils/code258/BaudotCode9.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- ITA2 Standard Tables ---- */
// 5-bit code index 0..31, letters shift
const char BaudotCode9::s_lettersTable[32] = {
    '\0', 'E', '\n', 'A', ' ', 'S', 'I', 'U',
    '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
    'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q',
    'O', 'B', 'G', '\0', 'M', 'X', 'V', '\0'
};
// 5-bit code index 0..31, figures shift
const char BaudotCode9::s_figuresTable[32] = {
    '\0', '3', '\n', '-', ' ', '\'', '8', '7',
    '\r', '2', '4', '7', ',', '!', ':', '(',
    '5', '"', ')', '2', '#', '6', '0', '1',
    '9', '?', '&', '\0', '.', '/', '=', '\0'
};

/* ---- Construction / Destruction ---- */

BaudotCode9::BaudotCode9(QObject *parent)
    : QObject(parent) { buildReverseTables(); }
BaudotCode9::~BaudotCode9() = default;

/* ---- Build reverse lookup ---- */

void BaudotCode9::buildReverseTables()
{
    m_lettersReverse.clear();
    m_figuresReverse.clear();
    for (int i = 0; i < 32; ++i) {
        char lc = s_lettersTable[i];
        if (lc != '\0' && lc != '\r' && lc != '\n')
            m_lettersReverse[QChar(lc).toUpper()] = static_cast<quint8>(i);
        char fc = s_figuresTable[i];
        if (fc != '\0' && fc != '\r' && fc != '\n')
            m_figuresReverse[QChar(fc)] = static_cast<quint8>(i);
    }
}

/* ---- Parity ---- */

quint8 BaudotCode9::computeParity(quint8 data5bit) const
{
    quint8 bits = data5bit & 0x1F;
    int count = 0;
    for (int i = 0; i < 5; ++i)
        if (bits & (1 << i)) count++;
    return static_cast<quint8>(count & 1);  // Even parity bit
}

bool BaudotCode9::checkParity(quint8 code6bit) const
{
    quint8 data = code6bit >> 1;
    quint8 parityBit = code6bit & 0x01;
    return parityBit == computeParity(data);
}

/* ---- Error correction heuristic ---- */

quint8 BaudotCode9::correctError(quint8 code6bit) const
{
    if (checkParity(code6bit)) return code6bit;
    // Try flipping each bit to find valid code
    for (int i = 0; i < 6; ++i) {
        quint8 candidate = code6bit ^ (1 << i);
        if (checkParity(candidate)) return candidate;
    }
    return code6bit;  // Uncorrectable, return as-is
}

/* ---- Encode single char ---- */

quint8 BaudotCode9::encodeChar(QChar ch, bool& needShift)
{
    needShift = false;
    quint8 code = 0xFF;
    // Try letters first
    QChar upper = ch.toUpper();
    if (m_lettersReverse.contains(upper)) {
        code = m_lettersReverse[upper];
        if (m_figureShift) needShift = true;  // Need to switch back
    } else if (m_figuresReverse.contains(ch)) {
        code = m_figuresReverse[ch];
        if (!m_figureShift) needShift = true;  // Need figures shift
    }
    return code;
}

/* ---- Encode ---- */

QVector<quint8> BaudotCode9::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint8> result;
    const quint8 LETTERS_SHIFT = 0x1F;  // ITA2: 11111
    const quint8 FIGURES_SHIFT = 0x1B;  // ITA2: 11011

    for (const QChar& ch : text) {
        bool needShift = false;
        quint8 code = encodeChar(ch, needShift);
        if (code == 0xFF) continue;  // Skip unsupported

        if (needShift) {
            if (m_figureShift) {
                // Switch to letters
                result.append(LETTERS_SHIFT);
                m_figureShift = false;
            } else {
                // Switch to figures
                result.append(FIGURES_SHIFT);
                m_figureShift = true;
            }
        }
        // Add parity bit
        quint8 withParity = (code << 1) | computeParity(code);
        result.append(withParity);
    }

    double elapsed = timer.elapsed();
    m_stats.numEncoded += text.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encodeCompleted(text.size(), result.size(), elapsed);
    return result;
}

/* ---- Decode single ---- */

BaudotCode9::DecodeResult BaudotCode9::decodeSingle(quint8 code) const
{
    DecodeResult result;
    result.rawCode = code;
    result.parityOk = checkParity(code);
    result.wasCorrected = false;

    quint8 data = code >> 1;
    if (!result.parityOk) {
        quint8 corrected = correctError(code);
        data = corrected >> 1;
        result.wasCorrected = true;
    }

    // Handle shift codes
    if (data == 0x1F) {
        result.character = QChar('\\');  // Letters shift marker
        return result;
    }
    if (data == 0x1B) {
        result.character = QChar('|');   // Figures shift marker
        return result;
    }

    if (m_figureShift) {
        char ch = s_figuresTable[data];
        result.character = (ch != '\0') ? QChar(ch) : QChar('?');
    } else {
        char ch = s_lettersTable[data];
        result.character = (ch != '\0') ? QChar(ch) : QChar('?');
    }
    return result;
}

/* ---- Decode ---- */

QString BaudotCode9::decode(const QVector<quint8>& codes)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    int corrected = 0;
    m_figureShift = false;

    for (quint8 code : codes) {
        DecodeResult dr = decodeSingle(code);
        if (dr.wasCorrected) corrected++;

        // Handle shift codes
        quint8 data = code >> 1;
        if (!dr.parityOk) data = correctError(code) >> 1;

        if (data == 0x1F) {
            m_figureShift = false;
            continue;
        }
        if (data == 0x1B) {
            m_figureShift = true;
            continue;
        }

        if (dr.character != '\\' && dr.character != '|')
            result.append(dr.character);
    }

    double elapsed = timer.elapsed();
    m_stats.numDecoded += codes.size();
    m_stats.numCorrected += corrected;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decodeCompleted(result.size(), corrected, elapsed);
    return result;
}

/* ---- State queries ---- */

bool BaudotCode9::isInFigureShift() const { return m_figureShift; }
void BaudotCode9::reset() { m_figureShift = false; }

/* ---- Reset ---- */

void BaudotCode9::resetStatistics()
{
    m_figureShift = false;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
