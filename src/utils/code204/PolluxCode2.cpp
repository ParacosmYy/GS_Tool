/**
 * @file PolluxCode2.cpp
 * @brief PolluxCode2 实现
 *
 * 实现Pollux密码变体：摩尔斯码填充分析、差分点划频率攻击、密钥恢复。
 */

#include "utils/code204/PolluxCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PolluxCode2::PolluxCode2(QObject *parent) : QObject(parent) {}
PolluxCode2::~PolluxCode2() = default;

/* ---- Standard Morse table ---- */

QMap<QChar, QString> PolluxCode2::standardMorseTable()
{
    QMap<QChar, QString> table;
    table['A'] = ".-";    table['B'] = "-...";  table['C'] = "-.-.";
    table['D'] = "-..";   table['E'] = ".";     table['F'] = "..-.";
    table['G'] = "--.";   table['H'] = "....";  table['I'] = "..";
    table['J'] = ".---";  table['K'] = "-.-";   table['L'] = ".-..";
    table['M'] = "--";    table['N'] = "-.";    table['O'] = "---";
    table['P'] = ".--.";  table['Q'] = "--.-";  table['R'] = ".-.";
    table['S'] = "...";   table['T'] = "-";     table['U'] = "..-";
    table['V'] = "...-";  table['W'] = ".--";   table['X'] = "-..-";
    table['Y'] = "-.--";  table['Z'] = "--..";  table['1'] = ".----";
    table['2'] = "..---"; table['3'] = "...--"; table['4'] = "....-";
    table['5'] = "....."; table['6'] = "-...."; table['7'] = "--...";
    table['8'] = "---.."; table['9'] = "----."; table['0'] = "-----";
    return table;
}

/* ---- Encrypt ---- */

QString PolluxCode2::encrypt(const QString& plaintext,
                              const QMap<QChar, QString>& morseTable,
                              const QVector<int>& keyMap) const
{
    // keyMap: maps 0..9 to {-1=dot, 0=sep, 1=dash}
    QString morse;
    for (int i = 0; i < plaintext.size(); ++i) {
        QChar ch = plaintext[i].toUpper();
        if (morseTable.contains(ch)) {
            morse += morseTable[ch];
            if (i < plaintext.size() - 1) morse += ' ';
        }
    }

    QString cipher;
    for (int i = 0; i < morse.size(); ++i) {
        QChar c = morse[i];
        int target = -2;
        if (c == '.') target = -1;      // dot
        else if (c == '-') target = 1;  // dash
        else if (c == ' ') target = 0;  // separator

        // Find all digits mapping to this symbol
        QVector<int> candidates;
        for (int d = 0; d < keyMap.size(); ++d)
            if (keyMap[d] == target) candidates.append(d);

        if (!candidates.isEmpty())
            cipher += QString::number(candidates[candidates.size() / 2]);
    }
    return cipher;
}

/* ---- Decrypt ---- */

QString PolluxCode2::decrypt(const QString& cipher,
                              const QMap<QChar, QString>& morseTable,
                              const QVector<int>& keyMap) const
{
    // Convert cipher digits to morse symbols
    QString morse;
    for (int i = 0; i < cipher.size(); ++i) {
        int digit = cipher[i].digitValue();
        if (digit >= 0 && digit < keyMap.size()) {
            int sym = keyMap[digit];
            if (sym == -1) morse += '.';
            else if (sym == 1) morse += '-';
            else if (sym == 0) morse += ' ';
        }
    }

    // Reverse lookup: morse -> character
    QMap<QString, QChar> reverseTable;
    for (auto it = morseTable.constBegin(); it != morseTable.constEnd(); ++it)
        reverseTable[it.value()] = it.key();

    QString result;
    QStringList tokens = morse.split(' ', Qt::SkipEmptyParts);
    for (const QString& tok : tokens) {
        if (reverseTable.contains(tok))
            result += reverseTable[tok];
    }
    return result;
}

/* ---- Padding analysis ---- */

QVector<double> PolluxCode2::paddingAnalysis(const QString& cipher) const
{
    // Analyze frequency of each digit (0-9)
    QVector<double> freq(10, 0.0);
    int total = cipher.size();
    if (total == 0) return freq;

    for (int i = 0; i < total; ++i) {
        int d = cipher[i].digitValue();
        if (d >= 0 && d < 10) freq[d]++;
    }
    for (int i = 0; i < 10; ++i)
        freq[i] /= total;

    // Compute deviation from uniform (1/10)
    double uniform = 0.1;
    for (int i = 0; i < 10; ++i)
        freq[i] = qFabs(freq[i] - uniform);

    return freq;
}

/* ---- Count dot/dash frequencies per digit ---- */

void PolluxCode2::countFrequencies(const QString& morse,
                                    QVector<double>& dotFreq,
                                    QVector<double>& dashFreq) const
{
    dotFreq.resize(10);
    dashFreq.resize(10);
    dotFreq.fill(0.0);
    dashFreq.fill(0.0);

    QVector<int> dotCount(10, 0), dashCount(10, 0);
    int dots = 0, dashes = 0;

    for (int i = 0; i < morse.size(); ++i) {
        int digit = morse[i].digitValue();
        if (digit < 0 || digit > 9) continue;
        // Heuristic: even positions tend to be dots, odd to dashes in Pollux
        if (i % 2 == 0) { dotCount[digit]++; dots++; }
        else { dashCount[digit]++; dashes++; }
    }

    for (int d = 0; d < 10; ++d) {
        dotFreq[d] = (dots > 0) ? dotCount[d] / static_cast<double>(dots) : 0.0;
        dashFreq[d] = (dashes > 0) ? dashCount[d] / static_cast<double>(dashes) : 0.0;
    }
}

/* ---- Frequency attack ---- */

QVector<int> PolluxCode2::frequencyAttack(const QString& cipher,
                                           const QMap<QChar, QString>& morseTable) const
{
    QElapsedTimer timer;
    timer.start();

    // Count digit frequencies in cipher
    QVector<int> digitCount(10, 0);
    for (int i = 0; i < cipher.size(); ++i) {
        int d = cipher[i].digitValue();
        if (d >= 0 && d < 10) digitCount[d]++;
    }

    // Build expected morse symbol ratio (dot:dash:sep)
    int totalDots = 0, totalDashes = 0;
    for (auto it = morseTable.constBegin(); it != morseTable.constEnd(); ++it) {
        for (const QChar& c : it.value()) {
            if (c == '.') totalDots++;
            else if (c == '-') totalDashes++;
        }
    }
    int totalSymbols = totalDots + totalDashes;

    // Sort digits by frequency (descending)
    QVector<int> sortedDigits(10);
    for (int i = 0; i < 10; ++i) sortedDigits[i] = i;
    std::sort(sortedDigits.begin(), sortedDigits.end(), [&](int a, int b) {
        return digitCount[a] > digitCount[b];
    });

    // Assign: most frequent -> dot, next -> dash, least -> separator
    QVector<int> keyMap(10, 0);  // default separator
    int dotAssign = qCeil(10.0 * totalDots / (totalSymbols + totalDots + 1));
    int dashAssign = qCeil(10.0 * totalDashes / (totalSymbols + totalDashes + 1));
    dotAssign = qBound(1, dotAssign, 4);
    dashAssign = qBound(1, dashAssign, 4);

    for (int i = 0; i < dotAssign && i < 10; ++i)
        keyMap[sortedDigits[i]] = -1;  // dot
    for (int i = dotAssign; i < dotAssign + dashAssign && i < 10; ++i)
        keyMap[sortedDigits[i]] = 1;   // dash
    // Remaining digits stay 0 (separator)

    return keyMap;
}

/* ---- Reset ---- */

void PolluxCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
