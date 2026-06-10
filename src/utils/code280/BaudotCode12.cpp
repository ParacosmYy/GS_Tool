/**
 * @file BaudotCode12.cpp
 * @brief BaudotCode12 实现
 *
 * 实现博多码：Murray码变体与字母/数字切换管理的ITA2兼容电报通信。
 */

#include "utils/code280/BaudotCode12.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

BaudotCode12::BaudotCode12(QObject *parent)
    : QObject(parent)
{
    buildTables();
}

BaudotCode12::~BaudotCode12() = default;

/* ---- Build ITA2 lookup tables ---- */

void BaudotCode12::buildTables()
{
    // ITA2 standard 32-character tables (5-bit codes 0..31)
    // Index = 5-bit code, value = character
    m_letterTable = QVector<QChar>(32, QChar());
    m_figureTable = QVector<QChar>(32, QChar());

    // ITA2 Letter shift table
    QChar letters[] = {
        QChar(),    QChar('E'), QChar('\n'), QChar('A'), QChar(' '), QChar('S'), QChar('I'), QChar('U'),
        QChar('\r'),QChar('D'), QChar('R'),  QChar('J'), QChar('N'), QChar('F'), QChar('C'), QChar('K'),
        QChar('T'), QChar('Z'), QChar('L'),  QChar('W'), QChar('H'), QChar('Y'), QChar('P'), QChar('Q'),
        QChar('O'), QChar('B'), QChar('G'),  QChar(),    QChar('M'), QChar('X'), QChar('V'), QChar()
    };
    // ITA2 Figure shift table
    QChar figures[] = {
        QChar(),    QChar('3'), QChar('\n'), QChar('-'), QChar(' '), QChar('\''),QChar('8'), QChar('7'),
        QChar('\r'),QChar('W'), QChar('4'),  QChar('7'), QChar(','), QChar('!'), QChar(':'), QChar('('),
        QChar('5'), QChar('"'), QChar(')'),  QChar('2'), QChar('#'), QChar('6'), QChar('0'), QChar('1'),
        QChar('9'), QChar('?'), QChar('&'),  QChar(),    QChar('.'), QChar('/'), QChar('='), QChar()
    };

    for (int i = 0; i < 32; ++i) {
        m_letterTable[i] = letters[i];
        m_figureTable[i] = figures[i];
    }

    // Murray variant: swap some figure assignments
    if (m_murray) {
        m_figureTable[11] = QChar('!');
        m_figureTable[20] = QChar('+');
    }

    // Build reverse lookup
    m_letterReverse.clear();
    m_figureReverse.clear();
    for (int i = 0; i < 32; ++i) {
        if (!m_letterTable[i].isNull())
            m_letterReverse[m_letterTable[i]] = static_cast<quint8>(i);
        if (!m_figureTable[i].isNull())
            m_figureReverse[m_figureTable[i]] = static_cast<quint8>(i);
    }
}

/* ---- Configuration ---- */

void BaudotCode12::setMurrayMode(bool murray)
{
    m_murray = murray;
    buildTables();
}

/* ---- Encode text to 5-bit Baudot codes ---- */

BaudotCode12::EncodeResult BaudotCode12::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    EncodeResult result;
    m_currentShift = Letters;

    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i].toUpper();

        // Try letter shift first
        if (m_letterReverse.contains(ch)) {
            if (m_currentShift != Letters) {
                result.fiveBitCodes.append(LTRS_CODE);
                result.shiftSequence.append(true);
                m_currentShift = Letters;
                result.numShifts++;
            }
            result.fiveBitCodes.append(m_letterReverse[ch]);
            result.shiftSequence.append(false);
        }
        // Try figure shift
        else if (m_figureReverse.contains(ch)) {
            if (m_currentShift != Figures) {
                result.fiveBitCodes.append(FIGS_CODE);
                result.shiftSequence.append(true);
                m_currentShift = Figures;
                result.numShifts++;
            }
            result.fiveBitCodes.append(m_figureReverse[ch]);
            result.shiftSequence.append(false);
        }
        // Unknown character: skip
    }

    double elapsed = timer.elapsed();
    m_stats.charsEncoded += text.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encodeDone(text.size(), result.fiveBitCodes.size(), elapsed);

    return result;
}

/* ---- Decode 5-bit Baudot codes to text ---- */

BaudotCode12::DecodeResult BaudotCode12::decode(const QVector<quint8>& codes)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    m_currentShift = Letters;

    for (quint8 code : codes) {
        if (code > 31) {
            result.numErrors++;
            continue;
        }

        // Handle shift codes
        if (code == LTRS_CODE) {
            m_currentShift = Letters;
            result.numShifts++;
            emit shiftChanged(Letters);
            continue;
        }
        if (code == FIGS_CODE) {
            m_currentShift = Figures;
            result.numShifts++;
            emit shiftChanged(Figures);
            continue;
        }

        // Decode based on current shift mode
        QChar ch;
        if (m_currentShift == Letters)
            ch = m_letterTable[code];
        else
            ch = m_figureTable[code];

        if (!ch.isNull())
            result.text.append(ch);
        else if (code != 0)
            result.numErrors++;
    }

    double elapsed = timer.elapsed();
    m_stats.charsDecoded += result.text.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decodeDone(codes.size(), result.text.size(), result.numErrors, elapsed);

    return result;
}

/* ---- Lookup helpers ---- */

QChar BaudotCode12::letterFromCode(quint8 code) const
{
    return (code < 32) ? m_letterTable[code] : QChar();
}

QChar BaudotCode12::figureFromCode(quint8 code) const
{
    return (code < 32) ? m_figureTable[code] : QChar();
}

/* ---- Code to wire state conversion ---- */

QVector<bool> BaudotCode12::codeToWire(quint8 code) const
{
    // Start bit (space=0) + 5 data bits (LSB first) + stop bit (mark=1)
    QVector<bool> wire(7, false);
    wire[0] = false;  // Start bit
    for (int i = 0; i < 5; ++i)
        wire[1 + i] = (code >> i) & 1;
    wire[6] = true;   // Stop bit
    return wire;
}

quint8 BaudotCode12::wireToCode(const QVector<bool>& wire) const
{
    if (wire.size() < 7) return 0;
    quint8 code = 0;
    for (int i = 0; i < 5; ++i)
        if (wire[1 + i]) code |= (1 << i);
    return code;
}

/* ---- Reset ---- */

void BaudotCode12::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_currentShift = Letters;
}
