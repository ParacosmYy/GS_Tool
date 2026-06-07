/**
 * @file BaudotCode2.cpp
 * @brief BaudotCode2 实现
 *
 * 实现博多码：ITA2字符集编解码、数字/字母移位状态机、5位编码。
 */

#include "utils/code210/BaudotCode2.h"

#include <QElapsedTimer>
#include <QBitArray>

/* ---- ITA2 Character Tables (5-bit codes 0-31) ---- */

const char* BaudotCode2::s_letterTable =
    "\0E\nA SIU\rDRJNFCKTZLWHYPQOBGMXV";  // Letters

const char* BaudotCode2::s_figureTable =
    "\03\n- \a87\r5\b4$,!:(5\")2#6'0@9=/"; // Figures approximated

/* ---- Construction / Destruction ---- */

BaudotCode2::BaudotCode2(QObject *parent) : QObject(parent) {}
BaudotCode2::~BaudotCode2() = default;

/* ---- Reverse 5 bits ---- */

quint8 BaudotCode2::reverseBits5(quint8 code)
{
    quint8 result = 0;
    for (int i = 0; i < 5; ++i) {
        result = (result << 1) | (code & 1);
        code >>= 1;
    }
    return result;
}

/* ---- Find code in table ---- */

int BaudotCode2::findCode(char ch, const char* table)
{
    for (int i = 0; i < 32; ++i) {
        if (table[i] == ch) return i;
    }
    return -1;
}

/* ---- Table lookup ---- */

QChar BaudotCode2::letterChar(quint8 code)
{
    if (code > 31) return QChar();
    char ch = s_letterTable[code];
    return (ch >= 32) ? QChar(ch) : QChar();
}

QChar BaudotCode2::figureChar(quint8 code)
{
    if (code > 31) return QChar();
    char ch = s_figureTable[code];
    return (ch >= 32) ? QChar(ch) : QChar();
}

/* ---- Encode text to 5-bit codes ---- */

QVector<quint8> BaudotCode2::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint8> result;
    m_state = LetterShift;

    for (int i = 0; i < text.size(); ++i) {
        QChar qc = text[i].toUpper();
        char ch = qc.toLatin1();

        // Try current shift first
        int code = -1;
        if (m_state == LetterShift) {
            code = findCode(ch, s_letterTable);
            if (code < 0) {
                // Try figure shift
                code = findCode(ch, s_figureTable);
                if (code >= 0) {
                    // Insert FIGS shift code (27)
                    result.append(27);
                    m_state = FigureShift;
                    m_stats.shiftTransitions++;
                }
            }
        } else {
            code = findCode(ch, s_figureTable);
            if (code < 0) {
                code = findCode(ch, s_letterTable);
                if (code >= 0) {
                    // Insert LTRS shift code (31)
                    result.append(31);
                    m_state = LetterShift;
                    m_stats.shiftTransitions++;
                }
            }
        }

        if (code >= 0) {
            result.append(static_cast<quint8>(code));
            m_stats.charsEncoded++;
        }
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Decode 5-bit codes to text ---- */

QString BaudotCode2::decode(const QVector<quint8>& codes)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    m_state = LetterShift;

    for (quint8 code : codes) {
        code &= 0x1F;  // Mask to 5 bits

        if (code == 27) {
            // FIGS shift
            m_state = FigureShift;
            m_stats.shiftTransitions++;
            emit shiftChanged(FigureShift);
            continue;
        }
        if (code == 31) {
            // LTRS shift
            m_state = LetterShift;
            m_stats.shiftTransitions++;
            emit shiftChanged(LetterShift);
            continue;
        }

        QChar ch;
        if (m_state == LetterShift) {
            ch = letterChar(code);
        } else {
            ch = figureChar(code);
        }

        if (ch.isNull()) {
            m_stats.decodeErrors++;
        } else {
            result.append(ch);
            m_stats.charsDecoded++;
        }
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decodeCompleted(m_stats.charsDecoded, m_stats.decodeErrors,
                         timer.elapsed());
    return result;
}

/* ---- Encode to packed bytes ---- */

QByteArray BaudotCode2::encodePacked(const QString& text)
{
    auto codes = encode(text);
    // Pack 5-bit codes into byte stream
    QByteArray result;
    int bitPos = 0;
    quint8 currentByte = 0;

    for (quint8 code : codes) {
        for (int b = 4; b >= 0; --b) {
            currentByte = (currentByte << 1) | ((code >> b) & 1);
            bitPos++;
            if (bitPos == 8) {
                result.append(static_cast<char>(currentByte));
                currentByte = 0;
                bitPos = 0;
            }
        }
    }

    // Flush remaining bits
    if (bitPos > 0) {
        currentByte <<= (8 - bitPos);
        result.append(static_cast<char>(currentByte));
    }
    return result;
}

/* ---- Decode packed bytes ---- */

QString BaudotCode2::decodePacked(const QByteArray& packed)
{
    // Unpack 5-bit codes from byte stream
    QVector<quint8> codes;
    int bitBuffer = 0;
    int bitCount = 0;

    for (char byte : packed) {
        bitBuffer = (bitBuffer << 8) | (static_cast<quint8>(byte));
        bitCount += 8;

        while (bitCount >= 5) {
            bitCount -= 5;
            quint8 code = (bitBuffer >> bitCount) & 0x1F;
            codes.append(code);
        }
    }

    return decode(codes);
}

/* ---- Getters ---- */

BaudotCode2::ShiftState BaudotCode2::shiftState() const { return m_state; }

/* ---- Reset ---- */

void BaudotCode2::resetState() { m_state = LetterShift; }

void BaudotCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    resetState();
}
