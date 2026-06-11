/**
 * @file BaudotCode13.cpp
 * @brief BaudotCode13 实现
 *
 * 实现博多码：移位状态自动化与字符集切换实现ITA2兼容电传打字机编码。
 */

#include "utils/code294/BaudotCode13.h"

#include <QElapsedTimer>
#include <QtGlobal>

/* ---- Construction / Destruction ---- */

BaudotCode13::BaudotCode13(QObject *parent)
    : QObject(parent)
{
    initTables();
}

BaudotCode13::~BaudotCode13() = default;

/* ---- Initialize ITA2 code tables ---- */

void BaudotCode13::initTables()
{
    // ITA2 standard letter set (32 entries, 5-bit codes 0-31)
    // Index = 5-bit code, value = character
    m_letterSet = {
        QChar('\0'), QChar('E'), QChar('\n'), QChar('A'), // 0-3
        QChar(' '),  QChar('S'), QChar('I'),    QChar('U'),    // 4-7
        QChar('\r'), QChar('D'), QChar('R'),    QChar('J'),    // 8-11
        QChar('N'),  QChar('F'), QChar('C'),    QChar('K'),    // 12-15
        QChar('T'),  QChar('Z'), QChar('L'),    QChar('W'),    // 16-19
        QChar('H'),  QChar('Y'), QChar('P'),    QChar('Q'),    // 20-23
        QChar('O'),  QChar('B'), QChar('G'),    QChar('\0'),   // 24-27 (27=FIGS)
        QChar('M'),  QChar('X'), QChar('V'),    QChar('\0')    // 28-31 (31=LETR)
    };

    // ITA2 figure set
    m_figureSet = {
        QChar('\0'), QChar('3'), QChar('\n'), QChar('-'), // 0-3
        QChar(' '),  QChar('\''),QChar('8'),    QChar('7'),   // 4-7
        QChar('\r'), QChar('W'), QChar('4'),    QChar('\a'),  // 8-11 (WRU=bell)
        QChar(','),  QChar('!'), QChar(':'),    QChar('('),   // 12-15
        QChar('5'),  QChar('"'), QChar(')'),    QChar('2'),   // 16-19
        QChar('#'),  QChar('6'), QChar('0'),    QChar('1'),   // 20-23
        QChar('9'),  QChar('?'), QChar('&'),    QChar('\0'),  // 24-27
        QChar('.'),  QChar('/'), QChar('='),    QChar('\0')   // 28-31
    };

    // Build reverse lookup tables
    for (int i = 0; i < 32; ++i) {
        QChar lc = m_letterSet[i];
        if (lc != QChar('\0'))
            m_letterReverse[lc] = static_cast<quint8>(i);

        QChar fc = m_figureSet[i];
        if (fc != QChar('\0'))
            m_figureReverse[fc] = static_cast<quint8>(i);
    }

    // Special: map some additional common characters
    m_figureReverse[QChar('$')] = 0x22; // Bell/pound
    m_letterReverse[QChar('\t')] = 0x04; // Space as tab equivalent
}

/* ---- Encode single character ---- */

BaudotCode13::EncodedBit BaudotCode13::encodeChar(QChar ch, ShiftMode currentMode) const
{
    EncodedBit result;
    result.newMode = currentMode;

    QChar upper = ch.toUpper();

    // Check letter set first
    if (m_letterReverse.contains(upper)) {
        if (currentMode != ShiftMode::Letter) {
            result.shiftNeeded = true;
            result.code5 = SHIFT_FIGURE; // Need LETTER shift first
            result.newMode = ShiftMode::Letter;
            return result;
        }
        result.code5 = m_letterReverse[upper];
        result.shiftNeeded = false;
        result.newMode = ShiftMode::Letter;
        return result;
    }

    // Check figure set
    if (m_figureReverse.contains(ch)) {
        if (currentMode != ShiftMode::Figure) {
            result.shiftNeeded = true;
            result.code5 = SHIFT_FIGURE;
            result.newMode = ShiftMode::Figure;
            return result;
        }
        result.code5 = m_figureReverse[ch];
        result.shiftNeeded = false;
        result.newMode = ShiftMode::Figure;
        return result;
    }

    // Unknown character - use space
    result.code5 = 0x04; // Space
    result.shiftNeeded = false;
    return result;
}

/* ---- Decode single 5-bit code ---- */

QChar BaudotCode13::decodeCode(quint8 code5, ShiftMode mode) const
{
    if (code5 >= 32) return QChar(' ');
    if (mode == ShiftMode::Letter)
        return m_letterSet[code5];
    return m_figureSet[code5];
}

/* ---- Encode string to Baudot codes ---- */

QVector<BaudotCode13::EncodedBit> BaudotCode13::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<EncodedBit> result;
    ShiftMode mode = ShiftMode::Letter;
    int shiftCount = 0;

    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i];

        // Try current mode first (avoid unnecessary shifts)
        bool inLetter = (m_letterReverse.contains(ch.toUpper()) ||
                         m_letterReverse.contains(ch));
        bool inFigure = (m_figureReverse.contains(ch));

        if (inLetter && !inFigure) {
            // Character only in letter set
            if (mode != ShiftMode::Letter) {
                EncodedBit shift;
                shift.code5 = SHIFT_LETTER;
                shift.shiftNeeded = true;
                shift.newMode = ShiftMode::Letter;
                result.append(shift);
                mode = ShiftMode::Letter;
                shiftCount++;
            }
            EncodedBit bit;
            bit.code5 = m_letterReverse.value(ch.toUpper(), 0x04);
            bit.shiftNeeded = false;
            bit.newMode = mode;
            result.append(bit);
        } else if (inFigure && !inLetter) {
            // Character only in figure set
            if (mode != ShiftMode::Figure) {
                EncodedBit shift;
                shift.code5 = SHIFT_FIGURE;
                shift.shiftNeeded = true;
                shift.newMode = ShiftMode::Figure;
                result.append(shift);
                mode = ShiftMode::Figure;
                shiftCount++;
            }
            EncodedBit bit;
            bit.code5 = m_figureReverse.value(ch, 0x04);
            bit.shiftNeeded = false;
            bit.newMode = mode;
            result.append(bit);
        } else if (inLetter) {
            // Ambiguous - use current mode
            quint8 code = (mode == ShiftMode::Letter)
                ? m_letterReverse.value(ch.toUpper(), 0x04)
                : m_figureReverse.value(ch, 0x04);
            EncodedBit bit;
            bit.code5 = code;
            bit.shiftNeeded = false;
            bit.newMode = mode;
            result.append(bit);
        } else {
            // Space fallback
            EncodedBit bit;
            bit.code5 = 0x04;
            bit.shiftNeeded = false;
            bit.newMode = mode;
            result.append(bit);
        }
    }

    double elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_stats.shiftTransitions += shiftCount;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeDone(text.size(), result.size(), shiftCount, elapsed);
    return result;
}

/* ---- Decode Baudot codes to string ---- */

QString BaudotCode13::decode(const QVector<quint8>& codes)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    ShiftMode mode = ShiftMode::Letter;

    for (quint8 code : codes) {
        if (code == SHIFT_FIGURE) {
            mode = ShiftMode::Figure;
            m_stats.shiftTransitions++;
            continue;
        }
        if (code == SHIFT_LETTER) {
            mode = ShiftMode::Letter;
            m_stats.shiftTransitions++;
            continue;
        }
        QChar ch = decodeCode(code, mode);
        if (ch != QChar('\0'))
            result.append(ch);
    }

    m_currentMode = mode;
    double elapsed = timer.elapsed();
    m_stats.totalDecodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return result;
}

/* ---- Reset ---- */

void BaudotCode13::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_currentMode = ShiftMode::Letter;
}
