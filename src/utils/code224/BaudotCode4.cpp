/**
 * @file BaudotCode4.cpp
 * @brief BaudotCode4 实现
 *
 * 实现CCITT-2博多码编解码、移位状态自动机、同步恢复。
 */

#include "utils/code224/BaudotCode4.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

BaudotCode4::BaudotCode4(QObject *parent) : QObject(parent)
{
    initTables();
}

BaudotCode4::~BaudotCode4() = default;

/* ---- Initialize CCITT-2 tables ---- */

void BaudotCode4::initTables()
{
    // CCITT-2 International Alphabet (32 entries)
    // Index 0-31, codes 0b00000 to 0b11111
    m_letterTable = {
        QChar('\0'), QChar('E'), QChar('\n'), QChar('A'), // 0-3
        QChar(' '),  QChar('S'), QChar('I'),     QChar('U'),     // 4-7
        QChar('\r'), QChar('D'), QChar('R'),     QChar('J'),     // 8-11
        QChar('N'),  QChar('F'), QChar('C'),     QChar('K'),     // 12-15
        QChar('T'),  QChar('Z'), QChar('L'),     QChar('W'),     // 16-19
        QChar('H'),  QChar('Y'), QChar('P'),     QChar('Q'),     // 20-23
        QChar('O'),  QChar('B'), QChar('G'),     QChar('Figures'),// 24-27
        QChar('M'),  QChar('X'), QChar('V'),     QChar('Letters') // 28-31
    };

    m_figureTable = {
        QChar('\0'), QChar('3'), QChar('\n'), QChar('-'),   // 0-3
        QChar(' '),  QChar('\''),QChar('8'),    QChar('7'),   // 4-7
        QChar('\r'), QChar('WrU'),QChar('4'),   QChar('\a'),  // 8-11
        QChar(','),  QChar('!'), QChar(':'),    QChar('('),   // 12-15
        QChar('5'),  QChar('"'), QChar(')'),    QChar('2'),   // 16-19
        QChar('#'),  QChar('6'), QChar('0'),    QChar('1'),   // 20-23
        QChar('9'),  QChar('?'), QChar('&'),    QChar('Figures'),// 24-27
        QChar('.'),  QChar('/'), QChar(';'),    QChar('Letters') // 28-31
    };

    // Build reverse lookups
    for (int i = 0; i < 32; ++i) {
        QChar lc = m_letterTable[i];
        if (lc.toLatin1() >= 'A' && lc.toLatin1() <= 'Z')
            m_letterLookup[lc] = i;
        if (lc == ' ') m_letterLookup[lc] = i;

        QChar fc = m_figureTable[i];
        if (fc.toLatin1() >= '0' && fc.toLatin1() <= '9')
            m_figureLookup[fc] = i;
        if (fc.toLatin1() >= ' ' && fc.toLatin1() < 127 && fc != QChar('Figures'))
            m_figureLookup[fc] = i;
    }
}

/* ---- Encode single character ---- */

QVector<quint8> BaudotCode4::encodeChar(QChar ch)
{
    QVector<quint8> result;
    QChar upper = ch.toUpper();

    // Check if letter
    if (m_letterLookup.contains(upper)) {
        if (m_shiftState == Figures) {
            m_shiftState = Letters;
            m_stats.shiftTransitions++;
            result.append(LETTERS_SHIFT);
            emit shiftChanged(Letters);
        }
        result.append(m_letterLookup[upper]);
    }
    // Check if figure
    else if (m_figureLookup.contains(ch)) {
        if (m_shiftState == Letters) {
            m_shiftState = Figures;
            m_stats.shiftTransitions++;
            result.append(FIGURES_SHIFT);
            emit shiftChanged(Figures);
        }
        result.append(m_figureLookup[ch]);
    }
    // Handle space in both modes
    else if (ch == ' ') {
        result.append(0x04);
    }

    return result;
}

/* ---- Decode single codeword ---- */

QChar BaudotCode4::decodeCodeWord(quint8 code)
{
    code &= 0x1F; // Mask to 5 bits

    if (code == LETTERS_SHIFT) {
        m_shiftState = Letters;
        m_stats.shiftTransitions++;
        emit shiftChanged(Letters);
        return QChar();
    }
    if (code == FIGURES_SHIFT) {
        m_shiftState = Figures;
        m_stats.shiftTransitions++;
        emit shiftChanged(Figures);
        return QChar();
    }

    if (m_shiftState == Letters) {
        return (code < m_letterTable.size()) ? m_letterTable[code] : QChar();
    } else {
        return (code < m_figureTable.size()) ? m_figureTable[code] : QChar();
    }
}

/* ---- Encode full text ---- */

QVector<quint8> BaudotCode4::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint8> result;
    for (int i = 0; i < text.size(); ++i) {
        QVector<quint8> cw = encodeChar(text[i]);
        for (quint8 c : cw) result.append(c);
    }

    m_stats.charsEncoded += text.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encodingCompleted(text.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Decode full stream ---- */

QString BaudotCode4::decode(const QVector<quint8>& codeWords)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    for (int i = 0; i < codeWords.size(); ++i) {
        QChar ch = decodeCodeWord(codeWords[i]);
        if (!ch.isNull()) result.append(ch);
    }

    m_stats.charsDecoded += codeWords.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Shift state accessors ---- */

BaudotCode4::ShiftState BaudotCode4::shiftState() const { return m_shiftState; }

void BaudotCode4::forceShiftState(ShiftState state)
{
    m_shiftState = state;
    m_stats.syncRecoveries++;
    emit shiftChanged(state);
}

/* ---- Add framing (start + 5 data + stop bits) ---- */

QByteArray BaudotCode4::addFraming(const QVector<quint8>& codeWords) const
{
    QByteArray framed;
    for (quint8 cw : codeWords) {
        // Start bit (0) + 5 data bits (LSB first) + 1.5 stop bits (1)
        framed.append(static_cast<char>(0)); // Start bit
        for (int b = 0; b < 5; ++b)
            framed.append(static_cast<char>((cw >> b) & 1));
        framed.append(static_cast<char>(1)); // Stop bit
        framed.append(static_cast<char>(1)); // Stop bit (1.5 -> 2)
    }
    return framed;
}

/* ---- Remove framing with sync recovery ---- */

QVector<quint8> BaudotCode4::removeFraming(const QByteArray& framed)
{
    QVector<quint8> result;
    int frameLen = 8; // start + 5 data + 2 stop
    int pos = 0;

    while (pos + frameLen <= framed.size()) {
        // Look for start bit (0)
        if (framed[pos] != 0) {
            pos++;
            m_stats.syncRecoveries++;
            emit syncRecovered(pos);
            continue;
        }

        // Extract 5 data bits (LSB first)
        quint8 code = 0;
        for (int b = 0; b < 5; ++b) {
            int bitPos = pos + 1 + b;
            if (bitPos < framed.size() && framed[bitPos] != 0)
                code |= (1 << b);
        }

        // Verify stop bits
        int stopPos = pos + 6;
        if (stopPos < framed.size() && framed[stopPos] == 0) {
            m_stats.framingErrors++;
            pos++;
            continue;
        }

        result.append(code);
        pos += frameLen;
    }
    return result;
}

/* ---- Reset ---- */

void BaudotCode4::resetStatistics()
{
    m_stats = Stats{};
    m_shiftState = Letters;
    m_timeSum = 0.0;
}
