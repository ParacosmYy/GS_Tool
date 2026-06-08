/**
 * @file BaudotCode3.cpp
 * @brief BaudotCode3 实现
 *
 * 实现Baudot/Murray/ITA2编码：移位状态机、无歧义解码、错误恢复。
 */

#include "utils/code216/BaudotCode3.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

BaudotCode3::BaudotCode3(QObject *parent) : QObject(parent)
{
    buildITA2Tables();
}

BaudotCode3::~BaudotCode3() = default;

/* ---- Configuration ---- */

void BaudotCode3::setVariant(int variant)
{
    m_variant = qBound(0, variant, 1);
    if (m_variant == 0) buildITA2Tables();
    else buildMurrayTables();
    m_shiftState = Letters;
}

/* ---- Build ITA2 tables (32 entries) ---- */

void BaudotCode3::buildITA2Tables()
{
    // ITA2 standard: 5-bit codes 0x00..0x1F
    m_lettersTable = {
        '\0', 'E', '\n', 'A', ' ', 'S', 'I', 'U',
        '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
        'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q',
        'O', 'B', 'G', '\0', 'M', 'X', 'V', '\0'
    };
    m_figuresTable = {
        '\0', '3', '\n', '-', ' ', '\'', '8', '7',
        '\r', 'WRU', '4', '\a', ',', '!', ':', '(',
        '5', '"', ')', '2', '#', '6', '0', '1',
        '9', '?', '&', '\0', '.', '/', '=', '\0'
    };

    // Build reverse lookup (256 chars)
    m_letterLookup.fill(0xFF, 256);
    m_figureLookup.fill(0xFF, 256);
    for (int i = 0; i < 32; ++i) {
        ushort lc = m_lettersTable[i].unicode();
        if (lc < 256) m_letterLookup[lc] = i;
        ushort fc = m_figuresTable[i].unicode();
        if (fc < 256) m_figureLookup[fc] = i;
    }
}

/* ---- Build Murray tables ---- */

void BaudotCode3::buildMurrayTables()
{
    // Murray variant: similar to ITA2 with minor differences
    m_lettersTable = {
        '\0', 'E', '\n', 'A', ' ', 'S', 'I', 'U',
        '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
        'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q',
        'O', 'B', 'G', '\0', 'M', 'X', 'V', '\0'
    };
    m_figuresTable = {
        '\0', '3', '\n', '-', ' ', '\'', '8', '7',
        '\r', '2', '4', '\a', ',', '!', ':', '(',
        '5', '"', ')', '2', '#', '6', '0', '1',
        '9', '?', '&', '\0', '.', '/', '=', '\0'
    };

    m_letterLookup.fill(0xFF, 256);
    m_figureLookup.fill(0xFF, 256);
    for (int i = 0; i < 32; ++i) {
        ushort lc = m_lettersTable[i].unicode();
        if (lc < 256) m_letterLookup[lc] = i;
        ushort fc = m_figuresTable[i].unicode();
        if (fc < 256) m_figureLookup[fc] = i;
    }
}

/* ---- Force shift ---- */

quint8 BaudotCode3::forceShift(ShiftState target)
{
    m_shiftState = target;
    // 0x1F = Letters shift, 0x1B = Figures shift
    return (target == Letters) ? 0x1F : 0x1B;
}

/* ---- Encode ---- */

QVector<quint8> BaudotCode3::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint8> result;
    ShiftState neededShift = Letters;

    for (int i = 0; i < text.size(); ++i) {
        ushort c = text[i].toUpper().unicode();
        if (c >= 256) continue;

        // Determine best shift state
        quint8 letterCode = (c < 256) ? m_letterLookup[c] : 0xFF;
        quint8 figureCode = (c < 256) ? m_figureLookup[c] : 0xFF;

        bool inLetters = (letterCode != 0xFF);
        bool inFigures = (figureCode != 0xFF);

        if (inLetters && (!inFigures || m_shiftState == Letters)) {
            if (m_shiftState != Letters)
                result.append(forceShift(Letters));
            result.append(letterCode & 0x1F);
        } else if (inFigures) {
            if (m_shiftState != Figures)
                result.append(forceShift(Figures));
            result.append(figureCode & 0x1F);
        }
    }

    m_stats.encodedChars += text.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Error recovery ---- */

BaudotCode3::DecodeResult BaudotCode3::recoverError(quint8 code, ErrorType type)
{
    DecodeResult r;
    r.character = '?';
    r.state = m_shiftState;
    r.error = type;
    r.wasShifted = false;
    m_stats.errorsRecovered++;
    return r;
}

/* ---- Decode single ---- */

BaudotCode3::DecodeResult BaudotCode3::decodeSingle(quint8 code)
{
    DecodeResult result;
    code &= 0x1F;  // Mask to 5 bits

    // Shift codes
    if (code == 0x1F) {
        m_shiftState = Letters;
        result.character = QChar();
        result.state = Letters;
        result.wasShifted = true;
        m_stats.shiftTransitions++;
        return result;
    }
    if (code == 0x1B) {
        m_shiftState = Figures;
        result.character = QChar();
        result.state = Figures;
        result.wasShifted = true;
        m_stats.shiftTransitions++;
        return result;
    }

    // Null code
    if (code == 0x00 || code == 0x1A) {
        result.character = QChar();
        result.state = m_shiftState;
        result.error = None;
        result.wasShifted = false;
        return result;
    }

    // Decode based on current shift state
    if (m_shiftState == Letters) {
        if (code < m_lettersTable.size())
            result.character = m_lettersTable[code];
        else
            return recoverError(code, InvalidCode);
    } else {
        if (code < m_figuresTable.size())
            result.character = m_figuresTable[code];
        else
            return recoverError(code, InvalidCode);
    }

    result.state = m_shiftState;
    result.error = None;
    result.wasShifted = false;
    return result;
}

/* ---- Decode ---- */

QString BaudotCode3::decode(const QVector<quint8>& codes)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    for (quint8 code : codes) {
        DecodeResult dr = decodeSingle(code);
        if (dr.error != None) {
            // Error recovery: try alternate shift state
            ShiftState saved = m_shiftState;
            m_shiftState = (saved == Letters) ? Figures : Letters;
            DecodeResult alt = decodeSingle(code);
            if (alt.error == None && !alt.wasShifted) {
                result.append(alt.character);
                m_shiftState = alt.state;
            } else {
                m_shiftState = saved;
                result.append('?');
            }
        } else if (!dr.wasShifted && dr.character.unicode() != 0) {
            result.append(dr.character);
        }
    }

    m_stats.decodedChars += result.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decodingCompleted(result.size(), m_stats.errorsRecovered, timer.elapsed());
    return result;
}

/* ---- Accessors ---- */

BaudotCode3::ShiftState BaudotCode3::currentShiftState() const
{
    return m_shiftState;
}

/* ---- Reset ---- */

void BaudotCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_shiftState = Letters;
}
