/**
 * @file BaudotCode5.cpp
 * @brief BaudotCode5 实现
 *
 * 实现博多码：ITA2国际/US TTY双模式自动检测与纠错。
 */

#include "utils/code230/BaudotCode5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

BaudotCode5::BaudotCode5(QObject *parent) : QObject(parent)
{
    initITA2Tables();
    buildReverseLookup();
}

BaudotCode5::~BaudotCode5() = default;

/* ---- Initialize ITA2 tables ---- */

void BaudotCode5::initITA2Tables()
{
    // 32 entries for ITA2 International (5-bit codes 0-31)
    m_lettersTable = QVector<QChar>(32, QChar());
    m_figuresTable = QVector<QChar>(32, QChar());

    // ITA2 International mapping
    m_lettersTable[0]  = QChar();     // blank
    m_lettersTable[1]  = 'E';
    m_lettersTable[2]  = QChar(0x0A); // line feed
    m_lettersTable[3]  = 'A';
    m_lettersTable[4]  = ' ';
    m_lettersTable[5]  = 'S';
    m_lettersTable[6]  = 'I';
    m_lettersTable[7]  = 'U';
    m_lettersTable[8]  = QChar(0x0D); // carriage return
    m_lettersTable[9]  = 'D';
    m_lettersTable[10] = 'R';
    m_lettersTable[11] = 'J';
    m_lettersTable[12] = 'N';
    m_lettersTable[13] = 'F';
    m_lettersTable[14] = 'C';
    m_lettersTable[15] = 'K';
    m_lettersTable[16] = 'T';
    m_lettersTable[17] = 'Z';
    m_lettersTable[18] = 'L';
    m_lettersTable[19] = 'W';
    m_lettersTable[20] = 'H';
    m_lettersTable[21] = 'Y';
    m_lettersTable[22] = 'P';
    m_lettersTable[23] = 'Q';
    m_lettersTable[24] = 'O';
    m_lettersTable[25] = 'B';
    m_lettersTable[26] = 'G';
    m_lettersTable[27] = QChar();     // FIGURES
    m_lettersTable[28] = 'M';
    m_lettersTable[29] = 'X';
    m_lettersTable[30] = 'V';
    m_lettersTable[31] = QChar();     // LETTERS

    // Figures mode
    m_figuresTable[0]  = QChar();
    m_figuresTable[1]  = '3';
    m_figuresTable[2]  = QChar(0x0A);
    m_figuresTable[3]  = '-';
    m_figuresTable[4]  = ' ';
    m_figuresTable[5]  = '\'';
    m_figuresTable[6]  = '8';
    m_figuresTable[7]  = '7';
    m_figuresTable[8]  = QChar(0x0D);
    m_figuresTable[9]  = QChar();     // WRU
    m_figuresTable[10] = '4';
    m_figuresTable[11] = QChar(0x07); // BELL
    m_figuresTable[12] = ',';
    m_figuresTable[13] = '!';
    m_figuresTable[14] = ':';
    m_figuresTable[15] = '(';
    m_figuresTable[16] = '5';
    m_figuresTable[17] = '"';
    m_figuresTable[18] = ')';
    m_figuresTable[19] = '2';
    m_figuresTable[20] = '#';
    m_figuresTable[21] = '6';
    m_figuresTable[22] = '0';
    m_figuresTable[23] = '1';
    m_figuresTable[24] = '9';
    m_figuresTable[25] = '?';
    m_figuresTable[26] = '&';
    m_figuresTable[27] = QChar();     // FIGURES
    m_figuresTable[28] = '.';
    m_figuresTable[29] = '/';
    m_figuresTable[30] = ';';
    m_figuresTable[31] = QChar();     // LETTERS
}

/* ---- Initialize US TTY tables ---- */

void BaudotCode5::initUsTtyTables()
{
    m_lettersTable = QVector<QChar>(32, QChar());
    m_figuresTable = QVector<QChar>(32, QChar());

    m_lettersTable[0]  = QChar();
    m_lettersTable[1]  = 'E';
    m_lettersTable[2]  = QChar(0x0A);
    m_lettersTable[3]  = 'A';
    m_lettersTable[4]  = ' ';
    m_lettersTable[5]  = 'S';
    m_lettersTable[6]  = 'I';
    m_lettersTable[7]  = 'U';
    m_lettersTable[8]  = QChar(0x0D);
    m_lettersTable[9]  = 'D';
    m_lettersTable[10] = 'R';
    m_lettersTable[11] = 'J';
    m_lettersTable[12] = 'N';
    m_lettersTable[13] = 'F';
    m_lettersTable[14] = 'C';
    m_lettersTable[15] = 'K';
    m_lettersTable[16] = 'T';
    m_lettersTable[17] = 'Z';
    m_lettersTable[18] = 'L';
    m_lettersTable[19] = 'W';
    m_lettersTable[20] = 'H';
    m_lettersTable[21] = 'Y';
    m_lettersTable[22] = 'P';
    m_lettersTable[23] = 'Q';
    m_lettersTable[24] = 'O';
    m_lettersTable[25] = 'B';
    m_lettersTable[26] = 'G';
    m_lettersTable[27] = QChar();
    m_lettersTable[28] = 'M';
    m_lettersTable[29] = 'X';
    m_lettersTable[30] = 'V';
    m_lettersTable[31] = QChar();

    m_figuresTable[0]  = QChar();
    m_figuresTable[1]  = '3';
    m_figuresTable[2]  = QChar(0x0A);
    m_figuresTable[3]  = '-';
    m_figuresTable[4]  = ' ';
    m_figuresTable[5]  = '\'';
    m_figuresTable[6]  = '8';
    m_figuresTable[7]  = '7';
    m_figuresTable[8]  = QChar(0x0D);
    m_figuresTable[9]  = '$';
    m_figuresTable[10] = '4';
    m_figuresTable[11] = QChar(0x07);
    m_figuresTable[12] = ',';
    m_figuresTable[13] = '!';
    m_figuresTable[14] = ':';
    m_figuresTable[15] = '(';
    m_figuresTable[16] = '5';
    m_figuresTable[17] = '"';
    m_figuresTable[18] = ')';
    m_figuresTable[19] = '2';
    m_figuresTable[20] = '#';
    m_figuresTable[21] = '6';
    m_figuresTable[22] = '0';
    m_figuresTable[23] = '1';
    m_figuresTable[24] = '9';
    m_figuresTable[25] = '?';
    m_figuresTable[26] = '&';
    m_figuresTable[27] = QChar();
    m_figuresTable[28] = '.';
    m_figuresTable[29] = '/';
    m_figuresTable[30] = ';';
    m_figuresTable[31] = QChar();
}

/* ---- Build reverse lookup ---- */

void BaudotCode5::buildReverseLookup()
{
    m_letterEncode = QVector<int>(128, -1);
    m_figureEncode = QVector<int>(128, -1);

    for (int i = 0; i < 32; ++i) {
        QChar lc = m_lettersTable[i];
        if (!lc.isNull() && lc.toLatin1() >= 0 && lc.toLatin1() < 128)
            m_letterEncode[static_cast<uchar>(lc.toLatin1())] = i;

        QChar fc = m_figuresTable[i];
        if (!fc.isNull() && fc.toLatin1() >= 0 && fc.toLatin1() < 128)
            m_figureEncode[static_cast<uchar>(fc.toLatin1())] = i;
    }
}

/* ---- Set variant ---- */

void BaudotCode5::setVariant(Variant variant)
{
    m_variant = variant;
    if (variant == UsTty)
        initUsTtyTables();
    else
        initITA2Tables();
    buildReverseLookup();
}

/* ---- Encode ---- */

QVector<quint8> BaudotCode5::encode(const QString& text) const
{
    QVector<quint8> result;
    ShiftState currentShift = Letters;
    bool needLettersShift = true;

    for (int i = 0; i < text.size(); ++i) {
        char ch = text[i].toUpper().toLatin1();
        if (ch < 0) continue;

        int letterCode = (ch < 128) ? m_letterEncode[static_cast<uchar>(ch)] : -1;
        int figureCode = (ch < 128) ? m_figureEncode[static_cast<uchar>(ch)] : -1;

        bool useLetters = (letterCode >= 0);
        int code = useLetters ? letterCode : figureCode;

        if (code < 0) continue;

        if (useLetters && currentShift != Letters) {
            result.append(LETTERS_SHIFT);
            currentShift = Letters;
        } else if (!useLetters && currentShift != Figures) {
            result.append(FIGURES_SHIFT);
            currentShift = Figures;
        }
        result.append(static_cast<quint8>(code));
    }
    return result;
}

/* ---- Decode single code word ---- */

QChar BaudotCode5::decodeCodeWord(quint8 codeWord)
{
    if (codeWord == LETTERS_SHIFT) {
        m_shiftState = Letters;
        emit shiftChanged(Letters);
        return QChar();
    }
    if (codeWord == FIGURES_SHIFT) {
        m_shiftState = Figures;
        emit shiftChanged(Figures);
        return QChar();
    }

    const QVector<QChar>& table = (m_shiftState == Letters) ? m_lettersTable : m_figuresTable;
    if (codeWord < 32) return table[codeWord];
    return QChar();
}

/* ---- Decode ---- */

QString BaudotCode5::decode(const QVector<quint8>& codeWords)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    m_shiftState = Letters;

    for (int i = 0; i < codeWords.size(); ++i) {
        QChar ch = decodeCodeWord(codeWords[i] & 0x1F);
        if (!ch.isNull()) result.append(ch);
    }

    m_stats.numDecoded += codeWords.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return result;
}

/* ---- Auto-detect variant ---- */

BaudotCode5::Variant BaudotCode5::detectVariant(const QVector<quint8>& sample) const
{
    int ita2Score = 0;
    int usTtyScore = 0;

    for (quint8 cw : sample) {
        cw &= 0x1F;
        if (cw == 9) {
            // Code 9: ITA2 = WRU, US TTY = '$'
            // '$' is more common in data -> bias US TTY
            usTtyScore++;
        }
    }
    return (usTtyScore > ita2Score) ? UsTty : ITA2International;
}

/* ---- Validate parity ---- */

bool BaudotCode5::validateParity(quint8 codeWord) const
{
    // Even parity: count set bits in lower 5 bits
    int bits = 0;
    for (int i = 0; i < 5; ++i)
        if (codeWord & (1 << i)) bits++;
    return (bits % 2 == 0);
}

/* ---- Find nearest valid code word ---- */

quint8 BaudotCode5::findNearestValid(quint8 codeWord) const
{
    quint5 best = codeWord;
    for (int bit = 0; bit < 5; ++bit) {
        quint8 candidate = codeWord ^ (1 << bit);
        if (validateParity(candidate)) return candidate;
    }
    return codeWord;
}

/* ---- Correct errors ---- */

QVector<quint8> BaudotCode5::correctErrors(const QVector<quint8>& codeWords)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint8> corrected = codeWords;
    for (int i = 0; i < corrected.size(); ++i) {
        quint8 cw = corrected[i] & 0x1F;
        if (!validateParity(cw)) {
            quint8 fixed = findNearestValid(cw);
            if (fixed != cw) {
                emit errorCorrected(i, cw, fixed);
                corrected[i] = fixed;
                m_stats.numErrorsCorrected++;
            } else {
                emit uncorrectableError(i, cw);
                m_stats.numUncorrectable++;
            }
        }
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return corrected;
}

/* ---- Accessors ---- */

BaudotCode5::ShiftState BaudotCode5::shiftState() const { return m_shiftState; }

/* ---- Reset ---- */

void BaudotCode5::resetStatistics()
{
    m_shiftState = Letters;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
