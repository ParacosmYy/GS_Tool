/**
 * @file BaudotCode6.cpp
 * @brief BaudotCode6 实现
 *
 * 实现博多码：ITA2字符集与移位状态管理。
 */

#include "utils/code238/BaudotCode6.h"

#include <QElapsedTimer>
#include <QMap>

/* ---- Construction / Destruction ---- */

BaudotCode6::BaudotCode6(QObject *parent) : QObject(parent)
{
    initTables();
}

BaudotCode6::~BaudotCode6() = default;

/* ---- Initialize ITA2 tables ---- */

void BaudotCode6::initTables()
{
    // ITA2 standard letters table (index = 5-bit code)
    m_lettersTable = {
        '\0', 'E', '\n', 'A', ' ', 'S', 'I', 'U',
        '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
        'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q',
        'O', 'B', 'G', '\0', 'M', 'X', 'V', '\0'
    };

    // ITA2 standard figures table
    m_figuresTable = {
        '\0', '3', '\n', '-', ' ', '\'', '8', '7',
        '\r', 'WRU', '4', 'BEL', ',', '!', ':', '(',
        '5', '"', ')', '2', '#', '6', '0', '1',
        '9', '?', '&', '\0', '.', '/', '=', '\0'
    };

    // Build reverse lookup tables
    m_lettersReverse.fill(-1, 128);
    m_figuresReverse.fill(-1, 128);

    for (int i = 0; i < 32; ++i) {
        QChar lc = m_lettersTable[i];
        if (lc.unicode() >= 32 && lc.unicode() < 128)
            m_lettersReverse[lc.toUpper().toLatin1()] = i;

        QChar fc = m_figuresTable[i];
        if (fc.unicode() >= 32 && fc.unicode() < 128)
            m_figuresReverse[fc.toLatin1()] = i;
    }

    // Special mappings
    m_lettersReverse[' '] = 0x04;
    m_figuresReverse[' '] = 0x04;
}

/* ---- Encode single character ---- */

QVector<quint8> BaudotCode6::encodeChar(QChar ch)
{
    QVector<quint8> result;
    char c = ch.toUpper().toLatin1();

    // Try letters mode first
    int letterCode = (c >= 0 && c < 128) ? m_lettersReverse[c] : -1;
    int figureCode = -1;

    // Check figures table for digits and punctuation
    static const QString figuresChars = "0123456789-!'\":()?./&=\",";
    if (figuresChars.contains(c))
        figureCode = (c >= 0 && c < 128) ? m_figuresReverse[c] : -1;

    if (letterCode >= 0 && letterCode != SHIFT_LETTERS && letterCode != SHIFT_FIGURES) {
        // Character is in letters table
        if (m_mode != Letters) {
            result.append(SHIFT_LETTERS);
            m_mode = Letters;
            m_stats.numShiftLetters++;
            emit modeChanged(Letters);
        }
        result.append(static_cast<quint8>(letterCode));
    } else if (figureCode >= 0) {
        // Character is in figures table
        if (m_mode != Figures) {
            result.append(SHIFT_FIGURES);
            m_mode = Figures;
            m_stats.numShiftFigures++;
            emit modeChanged(Figures);
        }
        result.append(static_cast<quint8>(figureCode));
    } else {
        // Unmapped: send space as placeholder
        result.append(0x04);
        m_stats.numErrors++;
    }

    return result;
}

/* ---- Encode ---- */

QVector<quint8> BaudotCode6::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint8> result;
    m_mode = Letters;  // Reset to letters mode at start

    for (const QChar& ch : text)
        result.append(encodeChar(ch));

    m_stats.numEncoded += text.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encodeCompleted(text.size(), result.size());
    return result;
}

/* ---- Decode ---- */

QString BaudotCode6::decode(const QVector<quint8>& codes)
{
    QElapsedTimer timer;
    timer.start();

    m_mode = Letters;  // Reset to letters mode
    QString result;

    for (quint8 code : codes) {
        code &= 0x1F;  // Mask to 5 bits

        if (code == SHIFT_LETTERS) {
            m_mode = Letters;
            m_stats.numShiftLetters++;
            emit modeChanged(Letters);
            continue;
        }
        if (code == SHIFT_FIGURES) {
            m_mode = Figures;
            m_stats.numShiftFigures++;
            emit modeChanged(Figures);
            continue;
        }

        QChar ch;
        if (m_mode == Letters) {
            if (code < m_lettersTable.size())
                ch = m_lettersTable[code];
        } else {
            if (code < m_figuresTable.size())
                ch = m_figuresTable[code];
        }

        if (ch.unicode() != 0) {
            result.append(ch);
        } else if (code != BLANK) {
            m_stats.numErrors++;
        }
    }

    m_stats.numDecoded += codes.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decodeCompleted(codes.size(), result);
    return result;
}

/* ---- Accessors ---- */

BaudotCode6::Mode BaudotCode6::currentMode() const { return m_mode; }
void BaudotCode6::resetMode() { m_mode = Letters; }

/* ---- Reset ---- */

void BaudotCode6::resetStatistics()
{
    m_mode = Letters;
    m_stats = Stats{}; m_timeSum = 0.0;
}
