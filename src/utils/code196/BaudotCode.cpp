/**
 * @file BaudotCode.cpp
 * @brief BaudotCode 实现
 *
 * 实现Baudot/ITA2电传编码：5位编码、数字/字母切换、MTBF误码检测。
 */

#include "utils/code196/BaudotCode.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- ITA2 Lookup Tables ---- */

const char BaudotCode::s_letters[32] = {
    '\0', 'E', '\n', 'A', ' ', 'S', 'I', 'U',
    '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
    'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q',
    'O', 'B', 'G', '\0', 'M', 'X', 'V', '\0'
};

const char BaudotCode::s_figures[32] = {
    '\0', '3', '\n', '-', ' ', '\'', '8', '7',
    '\r', 'W', '4', 'B', ',', '!', ':', '(',
    '5', '"', ')', '2', '#', '6', '0', '1',
    '9', '?', '&', '\0', '.', '/', '=', '\0'
};

QHash<QChar, quint8> BaudotCode::buildLetterMap()
{
    QHash<QChar, quint8> m;
    for (int i = 0; i < 32; ++i)
        if (s_letters[i] != '\0') m[QChar(s_letters[i])] = static_cast<quint8>(i);
    return m;
}

QHash<QChar, quint8> BaudotCode::buildFigureMap()
{
    QHash<QChar, quint8> m;
    for (int i = 0; i < 32; ++i)
        if (s_figures[i] != '\0') m[QChar(s_figures[i])] = static_cast<quint8>(i);
    return m;
}

const QHash<QChar, quint8> BaudotCode::s_charToLetter = buildLetterMap();
const QHash<QChar, quint8> BaudotCode::s_charToFigure = buildFigureMap();

/* ---- Construction / Destruction ---- */

BaudotCode::BaudotCode(QObject *parent) : QObject(parent) {}
BaudotCode::~BaudotCode() = default;

/* ---- Configuration ---- */

void BaudotCode::setMTBFThreshold(double t) { m_mtbfThreshold = qBound(0.0001, t, 1.0); }
void BaudotCode::setErrorDetectionEnabled(bool e) { m_errorDetection = e; }

/* ---- Encode single character ---- */

QPair<quint8, BaudotCode::ShiftState> BaudotCode::encodeChar(QChar ch, ShiftState currentShift) const
{
    QChar upper = ch.toUpper();

    // Check if character is in letter table
    if (s_charToLetter.contains(upper)) {
        quint8 code = s_charToLetter[upper];
        if (currentShift != Letters) {
            // Emit letter shift first
            return {code, Letters};
        }
        return {code, Letters};
    }

    // Check figure table
    if (s_charToFigure.contains(upper)) {
        quint8 code = s_charToFigure[upper];
        if (currentShift != Figures) {
            return {code, Figures};
        }
        return {code, Figures};
    }

    // Unknown character: emit space
    return {0x04, currentShift}; // space = 00100
}

/* ---- Decode single 5-bit value ---- */

QChar BaudotCode::decodeChar(quint8 code5bit, ShiftState shift) const
{
    code5bit &= 0x1F;  // mask to 5 bits
    char ch = (shift == Letters) ? s_letters[code5bit] : s_figures[code5bit];
    return (ch != '\0') ? QChar(ch) : QChar('?');
}

/* ---- Bit confidence ---- */

double BaudotCode::bitConfidence(quint8 code) const
{
    // Simple parity-based confidence
    int bits = 0;
    quint8 v = code & 0x1F;
    for (int i = 0; i < 5; ++i) bits += (v >> i) & 1;
    // Even parity -> higher confidence
    return (bits % 2 == 0) ? 0.98 : 0.90;
}

/* ---- Encode ---- */

QVector<quint8> BaudotCode::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint8> result;
    ShiftState shift = Letters;

    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i];

        // Determine required shift
        QChar upper = ch.toUpper();
        bool needsLetterShift = s_charToLetter.contains(upper) && !s_charToFigure.contains(upper);
        bool needsFigureShift = s_charToFigure.contains(upper) && !s_charToLetter.contains(upper);
        bool inBoth = s_charToLetter.contains(upper) && s_charToFigure.contains(upper);

        if (needsLetterShift && shift == Figures) {
            result.append(LETTER_SHIFT);
            shift = Letters;
        } else if (needsFigureShift && shift == Letters && !inBoth) {
            result.append(FIGURE_SHIFT);
            shift = Figures;
        }

        auto encoded = encodeChar(ch, shift);
        result.append(encoded.first);
        shift = encoded.second;
    }

    m_stats.totalEncodes++;
    m_stats.inputChars = text.size();
    m_stats.outputBits = result.size() * 5;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(text.size(), result.size() * 5, timer.elapsed());
    return result;
}

/* ---- Decode ---- */

QString BaudotCode::decode(const QVector<quint8>& code)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    ShiftState shift = Letters;

    for (int i = 0; i < code.size(); ++i) {
        quint8 v = code[i] & 0x1F;

        if (v == LETTER_SHIFT) {
            shift = Letters;
            continue;
        }
        if (v == FIGURE_SHIFT) {
            shift = Figures;
            continue;
        }

        QChar ch = decodeChar(v, shift);
        result += ch;
    }

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return result;
}

/* ---- Error probability ---- */

double BaudotCode::errorProbability(const QVector<quint8>& code) const
{
    if (code.isEmpty()) return 0.0;
    double totalProb = 0.0;
    for (int i = 0; i < code.size(); ++i) {
        double conf = bitConfidence(code[i]);
        totalProb += (1.0 - conf);
    }
    return totalProb / code.size();
}

/* ---- Detect errors ---- */

QVector<int> BaudotCode::detectErrors(const QVector<quint8>& code) const
{
    QVector<int> errors;
    if (!m_errorDetection) return errors;

    for (int i = 0; i < code.size(); ++i) {
        quint8 v = code[i] & 0x1F;
        double conf = bitConfidence(v);
        if (conf < 1.0 - m_mtbfThreshold)
            errors.append(i);
    }

    // Check for invalid consecutive shift codes
    for (int i = 1; i < code.size(); ++i) {
        quint8 prev = code[i - 1] & 0x1F;
        quint8 cur = code[i] & 0x1F;
        if (prev == LETTER_SHIFT && cur == LETTER_SHIFT)
            errors.append(i);
        if (prev == FIGURE_SHIFT && cur == FIGURE_SHIFT)
            errors.append(i);
    }

    return errors;
}

/* ---- Reset ---- */

void BaudotCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
