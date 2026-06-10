/**
 * @file BaudotCode10.cpp
 * @brief BaudotCode10 实现
 *
 * 实现博多码：FIGS/LETRS换档管理与CRC-5检错的5位电传编码。
 */

#include "utils/code266/BaudotCode10.h"

#include <QElapsedTimer>
#include <QtGlobal>

/* ---- ITA2 code tables (standard Baudot) ---- */

const quint8 BaudotCode10::FIGS_CODE = 0x1B;  // 27 = FIGS shift
const quint8 BaudotCode10::LETRS_CODE = 0x1F; // 31 = LETRS shift

const QVector<QChar> BaudotCode10::s_lettersTable = {
    QLatin1Char('\0'), QLatin1Char('E'), QLatin1Char('\n'), QLatin1Char('A'),
    QLatin1Char(' '),  QLatin1Char('S'), QLatin1Char('I'),  QLatin1Char('U'),
    QLatin1Char('\r'), QLatin1Char('D'), QLatin1Char('R'),  QLatin1Char('J'),
    QLatin1Char('N'),  QLatin1Char('F'), QLatin1Char('C'),  QLatin1Char('K'),
    QLatin1Char('T'),  QLatin1Char('Z'), QLatin1Char('L'),  QLatin1Char('W'),
    QLatin1Char('H'),  QLatin1Char('Y'), QLatin1Char('P'),  QLatin1Char('Q'),
    QLatin1Char('O'),  QLatin1Char('B'), QLatin1Char('G'),  QLatin1Char('M'),
    QLatin1Char('X'),  QLatin1Char('V'), QLatin1Char('V')   // 31 = LETRS
};

const QVector<QChar> BaudotCode10::s_figuresTable = {
    QLatin1Char('\0'), QLatin1Char('3'), QLatin1Char('\n'), QLatin1Char('-'),
    QLatin1Char(' '),  QLatin1Char('\''),QLatin1Char('8'),  QLatin1Char('7'),
    QLatin1Char('\r'), QLatin1Char('W'), QLatin1Char('4'),  QLatin1Char('B'),
    QLatin1Char(','),  QLatin1Char('!'), QLatin1Char(':'),  QLatin1Char('('),
    QLatin1Char('5'),  QLatin1Char('+'), QLatin1Char(')'),  QLatin1Char('2'),
    QLatin1Char('$'),  QLatin1Char('6'), QLatin1Char('0'),  QLatin1Char('1'),
    QLatin1Char('9'),  QLatin1Char('?'), QLatin1Char('&'),  QLatin1Char('.'),
    QLatin1Char('/'),  QLatin1Char('='), QLatin1Char('V')
};

/* ---- Construction / Destruction ---- */

BaudotCode10::BaudotCode10(QObject *parent)
    : QObject(parent) {}

BaudotCode10::~BaudotCode10() = default;

/* ---- Character lookup ---- */

int BaudotCode10::findLetter(QChar ch) const
{
    QChar upper = ch.toUpper();
    for (int i = 0; i < s_lettersTable.size(); ++i) {
        if (s_lettersTable[i] == upper) return i;
    }
    return 4; // Default: space
}

int BaudotCode10::findFigure(QChar ch) const
{
    for (int i = 0; i < s_figuresTable.size(); ++i) {
        if (s_figuresTable[i] == ch) return i;
    }
    return -1;
}

/* ---- Encode text to 5-bit code words ---- */

QVector<quint8> BaudotCode10::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint8> result;
    m_inFigShift = false;
    int shifts = 0;

    for (const QChar& ch : text) {
        // Try letters table first
        int letterIdx = findLetter(ch);
        int figureIdx = findFigure(ch);

        // Prefer figure shift for digits and punctuation
        bool isFigChar = (ch.isDigit() || ch == '.' || ch == ',' ||
                          ch == ':' || ch == ';' || ch == '!' || ch == '?' ||
                          ch == '(' || ch == ')' || ch == '-' || ch == '+' ||
                          ch == '/' || ch == '=' || ch == '$' || ch == '&' ||
                          ch == '\'' || ch == '"');

        if (isFigChar && figureIdx >= 0) {
            if (!m_inFigShift) {
                result.append(FIGS_CODE);
                m_inFigShift = true;
                shifts++;
            }
            result.append(static_cast<quint8>(figureIdx));
        } else if (letterIdx >= 0 && s_lettersTable[letterIdx] != QLatin1Char('\0')) {
            if (m_inFigShift) {
                result.append(LETRS_CODE);
                m_inFigShift = false;
                shifts++;
            }
            result.append(static_cast<quint8>(letterIdx));
        }
    }

    double elapsed = timer.elapsed();
    m_stats.encodedChars += text.size();
    m_stats.shiftCount += shifts;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encodingUpdated(text.size(), shifts, elapsed);
    return result;
}

/* ---- Decode 5-bit code words to text ---- */

QString BaudotCode10::decode(const QVector<quint8>& codeWords)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    m_inFigShift = false;
    int shifts = 0;

    for (quint8 code : codeWords) {
        if (code == FIGS_CODE) {
            m_inFigShift = true;
            shifts++;
            continue;
        }
        if (code == LETRS_CODE) {
            m_inFigShift = false;
            shifts++;
            continue;
        }

        int idx = static_cast<int>(code);
        if (idx < 0 || idx >= s_lettersTable.size()) continue;

        if (m_inFigShift) {
            if (idx < s_figuresTable.size())
                result.append(s_figuresTable[idx]);
        } else {
            result.append(s_lettersTable[idx]);
        }
    }

    double elapsed = timer.elapsed();
    m_stats.decodedChars += result.size();
    m_stats.shiftCount += shifts;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- CRC-5 computation (polynomial x^5 + x^3 + 1, 0x14) ---- */

quint8 BaudotCode10::computeCRC5(const QVector<quint8>& data) const
{
    quint8 crc = 0x1F; // Initialize to all ones (5-bit)
    const quint8 poly = 0x14; // x^5 + x^3 + 1

    for (quint8 byte : data) {
        // Only use lower 5 bits
        quint8 val = byte & 0x1F;
        for (int i = 4; i >= 0; --i) {
            bool msb = (crc >> 4) & 1;
            crc <<= 1;
            if ((val >> i) & 1) crc ^= 1;
            if (msb) crc ^= poly;
            crc &= 0x1F;
        }
    }
    return crc;
}

/* ---- Verify CRC-5 ---- */

bool BaudotCode10::verifyCRC5(const QVector<quint8>& dataWithCRC) const
{
    if (dataWithCRC.size() < 2) return false;

    // Last code word contains CRC-5
    QVector<quint8> payload = dataWithCRC.mid(0, dataWithCRC.size() - 1);
    quint8 expected = dataWithCRC.last() & 0x1F;
    quint8 actual = computeCRC5(payload);
    return actual == expected;
}

/* ---- Pack 5-bit code words into byte stream ---- */

QByteArray BaudotCode10::packBits(const QVector<quint8>& codeWords) const
{
    QByteArray result;
    int bitBuffer = 0;
    int bitCount = 0;

    for (quint8 code : codeWords) {
        bitBuffer = (bitBuffer << 5) | (code & 0x1F);
        bitCount += 5;

        while (bitCount >= 8) {
            bitCount -= 8;
            result.append(static_cast<char>((bitBuffer >> bitCount) & 0xFF));
        }
    }

    // Flush remaining bits
    if (bitCount > 0) {
        result.append(static_cast<char>((bitBuffer << (8 - bitCount)) & 0xFF));
    }
    return result;
}

/* ---- Unpack byte stream to 5-bit code words ---- */

QVector<quint8> BaudotCode10::unpackBits(const QByteArray& bytes,
                                           int numCodeWords) const
{
    QVector<quint8> result;
    int bitBuffer = 0;
    int bitCount = 0;
    int extracted = 0;

    for (char byte : bytes) {
        bitBuffer = (bitBuffer << 8) | (static_cast<quint8>(byte));
        bitCount += 8;

        while (bitCount >= 5 && extracted < numCodeWords) {
            bitCount -= 5;
            result.append(static_cast<quint8>((bitBuffer >> bitCount) & 0x1F));
            extracted++;
        }
    }
    return result;
}

/* ---- Reset ---- */

void BaudotCode10::resetStatistics()
{
    m_inFigShift = false;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
