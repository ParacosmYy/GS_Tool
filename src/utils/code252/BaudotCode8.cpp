/**
 * @file BaudotCode8.cpp
 * @brief BaudotCode8 实现
 *
 * 实现博多码：ITA2国际字母表、Unicode映射与自动Shift检测启发式。
 */

#include "utils/code252/BaudotCode8.h"

#include <QElapsedTimer>
#include <QMap>

/* ---- Construction / Destruction ---- */

BaudotCode8::BaudotCode8(QObject *parent) : QObject(parent)
{
    buildTables();
}
BaudotCode8::~BaudotCode8() = default;

/* ---- Build ITA2 international alphabet tables ---- */

void BaudotCode8::buildTables()
{
    // ITA2 International Alphabet: 32 entries (5-bit, codes 0..31)
    // Index = 5-bit code value
    m_letterTable = QVector<QChar>(32, QChar());
    m_figureTable = QVector<QChar>(32, QChar());

    // Code:  0   1   2   3   4   5   6   7   8   9
    auto L = [&](int code, const QString& ch) { m_letterTable[code] = ch[0]; };
    auto F = [&](int code, const QString& ch) { m_figureTable[code] = ch[0]; };

    // ITA2 Letters row
    L(0,  QChar());   // Blank/NUL
    L(1,  QStringLiteral("E"));
    L(2,  QStringLiteral("\n"));   // Line feed
    L(3,  QStringLiteral("A"));
    L(4,  QStringLiteral(" "));
    L(5,  QStringLiteral("S"));
    L(6,  QStringLiteral("I"));
    L(7,  QStringLiteral("U"));
    L(8,  QStringLiteral("\r"));   // Carriage return
    L(9,  QStringLiteral("D"));
    L(10, QStringLiteral("R"));
    L(11, QStringLiteral("J"));
    L(12, QStringLiteral("N"));
    L(13, QStringLiteral("F"));
    L(14, QStringLiteral("C"));
    L(15, QStringLiteral("K"));
    L(16, QStringLiteral("T"));
    L(17, QStringLiteral("Z"));
    L(18, QStringLiteral("L"));
    L(19, QStringLiteral("W"));
    L(20, QStringLiteral("H"));
    L(21, QStringLiteral("Y"));
    L(22, QStringLiteral("P"));
    L(23, QStringLiteral("Q"));
    L(24, QStringLiteral("O"));
    L(25, QStringLiteral("B"));
    L(26, QStringLiteral("G"));
    L(27, QChar());   // FIGURES shift
    L(28, QStringLiteral("M"));
    L(29, QStringLiteral("X"));
    L(30, QStringLiteral("V"));
    L(31, QChar());   // LETTERS shift

    // ITA2 Figures row
    F(0,  QChar());   // Blank/NUL
    F(1,  QStringLiteral("3"));
    F(2,  QStringLiteral("\n"));
    F(3,  QStringLiteral("-"));
    F(4,  QStringLiteral(" "));
    F(5,  QStringLiteral("'"));
    F(6,  QStringLiteral("8"));
    F(7,  QStringLiteral("7"));
    F(8,  QStringLiteral("\r"));
    F(9,  QStringLiteral("WRU"));  // Who Are You (use 'D' fig)
    F(9,  QStringLiteral("D"));    // Fallback printable
    F(10, QStringLiteral("4"));
    F(11, QStringLiteral("!"));    // Bell in some variants
    F(12, QStringLiteral(","));
    F(13, QStringLiteral("!"));
    F(14, QStringLiteral(":"));
    F(15, QStringLiteral("("));
    F(16, QStringLiteral("5"));
    F(17, QStringLiteral("\""));
    F(18, QStringLiteral(")"));
    F(19, QStringLiteral("2"));
    F(20, QStringLiteral("#"));    // In some variants £
    F(21, QStringLiteral("6"));
    F(22, QStringLiteral("0"));
    F(23, QStringLiteral("1"));
    F(24, QStringLiteral("9"));
    F(25, QStringLiteral("?"));
    F(26, QStringLiteral("&"));
    F(27, QChar());   // FIGURES shift
    F(28, QStringLiteral("."));
    F(29, QStringLiteral("/"));
    F(30, QStringLiteral("="));
    F(31, QChar());   // LETTERS shift
}

/* ---- Lookup character -> 5-bit code ---- */

int BaudotCode8::lookupChar(QChar ch, ShiftState state) const
{
    const QVector<QChar>& table = (state == Letters) ? m_letterTable : m_figureTable;
    for (int i = 0; i < table.size(); ++i) {
        if (table[i] == ch) return i;
    }
    return -1; // Not found in this shift mode
}

/* ---- Lookup 5-bit code -> character ---- */

QChar BaudotCode8::lookupCode(quint8 code, ShiftState state) const
{
    if (code >= 32) return QChar();
    const QVector<QChar>& table = (state == Letters) ? m_letterTable : m_figureTable;
    return table[code];
}

/* ---- Auto-detect best shift state for a character ---- */

BaudotCode8::ShiftState BaudotCode8::detectShift(QChar ch) const
{
    // Heuristic: check if character is in letters table
    int inLetters = lookupChar(ch, Letters);
    int inFigures = lookupChar(ch, Figures);

    if (inLetters >= 0 && inFigures < 0) return Letters;
    if (inFigures >= 0 && inLetters < 0) return Figures;
    // Ambiguous or not found: prefer current state
    return m_shiftState;
}

/* ---- Set initial shift state ---- */

void BaudotCode8::setInitialShift(ShiftState state)
{
    m_shiftState = state;
}

/* ---- Enable/disable auto-shift detection ---- */

void BaudotCode8::setAutoShiftDetection(bool enabled)
{
    m_autoShift = enabled;
}

/* ---- Encode Unicode string to Baudot codes ---- */

QVector<quint8> BaudotCode8::encode(const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint8> result;
    result.reserve(text.size() * 2); // Worst case: shift per char
    int shiftCount = 0;

    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i].toUpper();

        // Determine needed shift state
        ShiftState needed = m_autoShift ? detectShift(ch) : m_shiftState;

        // Emit shift if needed
        if (needed != m_shiftState) {
            if (needed == Figures) {
                result.append(SHIFT_TO_FIGURES);
            } else {
                result.append(SHIFT_TO_LETTERS);
            }
            m_shiftState = needed;
            shiftCount++;
        }

        int code = lookupChar(ch, m_shiftState);
        if (code >= 0) {
            result.append(static_cast<quint8>(code));
        } else {
            // Character not representable: try other shift
            ShiftState other = (m_shiftState == Letters) ? Figures : Letters;
            code = lookupChar(ch, other);
            if (code >= 0) {
                result.append(other == Figures ? SHIFT_TO_FIGURES : SHIFT_TO_LETTERS);
                m_shiftState = other;
                shiftCount++;
                result.append(static_cast<quint8>(code));
            }
            // else: skip unrepresentable character
            m_stats.numErrors++;
        }
    }

    m_stats.numEncoded += text.size();
    m_stats.numShifts += shiftCount;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encodeCompleted(text.size(), shiftCount, timer.elapsed());
    return result;
}

/* ---- Decode Baudot codes to Unicode string ---- */

QString BaudotCode8::decode(const QVector<quint8>& codes)
{
    QElapsedTimer timer;
    timer.start();

    QString result;
    result.reserve(codes.size());
    ShiftState currentShift = Letters;
    int shiftCount = 0;

    for (quint8 code : codes) {
        if (code >= 32) {
            m_stats.numErrors++;
            continue;
        }

        // Check for shift codes
        if (code == SHIFT_TO_FIGURES) {
            currentShift = Figures;
            shiftCount++;
            continue;
        }
        if (code == SHIFT_TO_LETTERS) {
            currentShift = Letters;
            shiftCount++;
            continue;
        }

        QChar ch = lookupCode(code, currentShift);
        if (!ch.isNull()) {
            result.append(ch);
        }
    }

    m_shiftState = currentShift;
    m_stats.numDecoded += codes.size();
    m_stats.numShifts += shiftCount;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decodeCompleted(codes.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Reset ---- */

void BaudotCode8::resetStatistics()
{
    m_shiftState = Letters;
    m_stats = Stats{};
    m_timeSum = 0.0;
}
