/**
 * @file BaudotCode11.cpp
 * @brief BaudotCode11 实现
 *
 * 实现博多码：CCITT国际电报字母表2与空格回退标准5比特电报编码。
 */

#include "utils/code272/BaudotCode11.h"

#include <QElapsedTimer>
#include <QtMath>

/* ---- Construction / Destruction ---- */

BaudotCode11::BaudotCode11(QObject *parent)
    : QObject(parent)
{
    buildTables();
}

BaudotCode11::~BaudotCode11() = default;

/* ---- Configuration ---- */

void BaudotCode11::setUnshiftOnSpace(bool enabled) { m_unshiftOnSpace = enabled; }

/* ---- Build ITA2 lookup tables ---- */

void BaudotCode11::buildTables()
{
    // ITA2 Letters table (32 entries, 5-bit code = index)
    m_lettersTable = QVector<QChar>(32, QChar());
    // Code 0-31 for Letters mode
    m_lettersTable[0x00] = QChar();     // Blank/NA
    m_lettersTable[0x01] = u'E';
    m_lettersTable[0x02] = u'\n';       // Line Feed
    m_lettersTable[0x03] = u'A';
    m_lettersTable[0x04] = u' ';        // Space
    m_lettersTable[0x05] = u'S';
    m_lettersTable[0x06] = u'I';
    m_lettersTable[0x07] = u'U';
    m_lettersTable[0x08] = u'\r';       // Carriage Return
    m_lettersTable[0x09] = u'D';
    m_lettersTable[0x0A] = u'R';
    m_lettersTable[0x0B] = u'J';
    m_lettersTable[0x0C] = u'N';
    m_lettersTable[0x0D] = u'F';
    m_lettersTable[0x0E] = u'C';
    m_lettersTable[0x0F] = u'K';
    m_lettersTable[0x10] = u'T';
    m_lettersTable[0x11] = u'Z';
    m_lettersTable[0x12] = u'L';
    m_lettersTable[0x13] = u'W';
    m_lettersTable[0x14] = u'H';
    m_lettersTable[0x15] = u'Y';
    m_lettersTable[0x16] = u'P';
    m_lettersTable[0x17] = u'Q';
    m_lettersTable[0x18] = u'O';
    m_lettersTable[0x19] = u'B';
    m_lettersTable[0x1A] = u'G';
    m_lettersTable[0x1B] = QChar();     // FIGS shift
    m_lettersTable[0x1C] = u'M';
    m_lettersTable[0x1D] = u'X';
    m_lettersTable[0x1E] = u'V';
    m_lettersTable[0x1F] = QChar();     // LTRS shift

    // ITA2 Figures table
    m_figuresTable = QVector<QChar>(32, QChar());
    m_figuresTable[0x00] = QChar();     // Blank
    m_figuresTable[0x01] = u'3';
    m_figuresTable[0x02] = u'\n';
    m_figuresTable[0x03] = u'-';
    m_figuresTable[0x04] = u' ';
    m_figuresTable[0x05] = u'\'';
    m_figuresTable[0x06] = u'8';
    m_figuresTable[0x07] = u'7';
    m_figuresTable[0x08] = u'\r';
    m_figuresTable[0x09] = u'W';        // WRU (Who Are You)
    m_figuresTable[0x0A] = u'4';
    m_figuresTable[0x0B] = u'B';        // BELL
    m_figuresTable[0x0C] = u',';
    m_figuresTable[0x0D] = u'!';
    m_figuresTable[0x0E] = u':';
    m_figuresTable[0x0F] = u'(';
    m_figuresTable[0x10] = u'5';
    m_figuresTable[0x11] = u'"';
    m_figuresTable[0x12] = u')';
    m_figuresTable[0x13] = u'2';
    m_figuresTable[0x14] = u'#';
    m_figuresTable[0x15] = u'6';
    m_figuresTable[0x16] = u'0';
    m_figuresTable[0x17] = u'1';
    m_figuresTable[0x18] = u'9';
    m_figuresTable[0x19] = u'?';
    m_figuresTable[0x1A] = u'&';
    m_figuresTable[0x1B] = QChar();     // FIGS
    m_figuresTable[0x1C] = u'.';
    m_figuresTable[0x1D] = u'/';
    m_figuresTable[0x1E] = u';';
    m_figuresTable[0x1F] = QChar();     // LTRS

    // Build reverse lookup
    m_letterToCode.fill(0, 128);
    m_figureToCode.fill(0, 128);
    for (int i = 0; i < 32; ++i) {
        QChar lc = m_lettersTable[i];
        if (!lc.isNull() && lc.toLatin1() >= 0)
            m_letterToCode[static_cast<int>(lc.toUpper().toLatin1())] = i;
        QChar fc = m_figuresTable[i];
        if (!fc.isNull() && fc.toLatin1() >= 0)
            m_figureToCode[static_cast<int>(fc.toLatin1())] = i;
    }
}

/* ---- Encode single character ---- */

QVector<quint8> BaudotCode11::encodeChar(QChar ch)
{
    QVector<quint8> result;
    char c = ch.toUpper().toLatin1();

    // Check if it's a figure character
    bool isFigure = (c >= '0' && c <= '9') || c == '-' || c == '\'' ||
                    c == ',' || c == '!' || c == ':' || c == '(' ||
                    c == ')' || c == '"' || c == '#' || c == '?' ||
                    c == '&' || c == '.' || c == '/';

    if (isFigure) {
        if (m_shift != Figures) {
            result.append(SHIFT_FIGURES);
            m_shift = Figures;
            m_stats.numShifts++;
        }
        result.append(m_figureToCode[static_cast<int>(c)]);
    } else {
        if (m_shift != Letters) {
            result.append(SHIFT_LETTERS);
            m_shift = Letters;
            m_stats.numShifts++;
        }
        result.append(m_letterToCode[static_cast<int>(c)]);
    }
    return result;
}

/* ---- Encode string ---- */

QVector<quint8> BaudotCode11::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint8> result;
    m_shift = Letters;
    m_stats.numShifts = 0;

    for (const QChar& ch : text) {
        auto codes = encodeChar(ch);
        for (quint8 code : codes)
            result.append(code);

        // Unshift-on-space: return to Letters mode after space
        if (m_unshiftOnSpace && ch == u' ') {
            m_shift = Letters;
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numEncoded += text.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encodingDone(result.size(), m_stats.numShifts, elapsed);

    return result;
}

/* ---- Decode codewords ---- */

QString BaudotCode11::decode(const QVector<quint8>& codewords)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    m_shift = Letters;
    m_stats.numShifts = 0;

    for (quint8 code : codewords) {
        code = code & 0x1F;  // Mask to 5 bits

        if (code == SHIFT_FIGURES) {
            m_shift = Figures;
            m_stats.numShifts++;
            continue;
        }
        if (code == SHIFT_LETTERS) {
            m_shift = Letters;
            m_stats.numShifts++;
            continue;
        }

        QChar ch = (m_shift == Letters) ? m_lettersTable[code] : m_figuresTable[code];
        if (!ch.isNull()) {
            result.append(ch);

            // Unshift-on-space
            if (m_unshiftOnSpace && ch == u' ')
                m_shift = Letters;
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numDecoded += codewords.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decodingDone(result.size(), m_stats.numShifts, elapsed);

    return result;
}

/* ---- Accessors ---- */

BaudotCode11::ShiftState BaudotCode11::shiftState() const { return m_shift; }
void BaudotCode11::resetShift() { m_shift = Letters; }

/* ---- Reset ---- */

void BaudotCode11::resetStatistics()
{
    m_shift = Letters;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
