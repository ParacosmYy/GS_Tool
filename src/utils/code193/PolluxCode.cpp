/**
 * @file PolluxCode.cpp
 * @brief PolluxCode 实现
 *
 * 实现Pollux密码：莫尔斯电码变长编码、点划频率密码分析、密钥推断。
 */

#include "utils/code193/PolluxCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PolluxCode::PolluxCode(QObject *parent) : QObject(parent)
{
    initMorseTable();
    initDefaultKey();
}

PolluxCode::~PolluxCode() = default;

/* ---- Initialize Morse table ---- */

void PolluxCode::initMorseTable()
{
    // Standard Morse code: A-Z, 0-9
    static const QVector<QPair<QChar, QString>> table = {
        {'A', ".-"},   {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
        {'E', "."},    {'F', "..-."}, {'G', "--."},  {'H', "...."},
        {'I', ".."},   {'J', ".---"}, {'K', "-.-"},  {'L', ".-.."},
        {'M', "--"},   {'N', "-."},   {'O', "---"},  {'P', ".--."},
        {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
        {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"},
        {'Y', "-.--"}, {'Z', "--.."},
        {'0', "-----"},{'1', ".----"},{'2', "..---"},{'3', "...--"},
        {'4', "....-"},{'5', "....."},{'6', "-...."},{'7', "--..."},
        {'8', "---.."},{'9', "----."},
    };
    for (const auto& p : table) {
        m_morseTable[p.second] = p.first;
        m_charToMorse[p.first] = p.second;
    }
}

/* ---- Initialize default Pollux key ---- */

void PolluxCode::initDefaultKey()
{
    // Default: dot=1,4,7  dash=2,5,8  sep=3,6,9,0
    m_key['.'] = {1, 4, 7};
    m_key['-'] = {2, 5, 8};
    m_key[' '] = {3, 6, 9, 0};
}

/* ---- Configuration ---- */

void PolluxCode::setKey(const QMap<QChar, QVector<int>>& key)
{
    m_key = key;
}

/* ---- To Morse ---- */

QString PolluxCode::toMorse(const QString& text) const
{
    QString result;
    QString upper = text.toUpper();
    for (int i = 0; i < upper.size(); ++i) {
        QChar c = upper[i];
        if (m_charToMorse.contains(c)) {
            result += m_charToMorse[c];
            if (i < upper.size() - 1) result += ' ';
        }
    }
    return result;
}

/* ---- From Morse ---- */

QString PolluxCode::fromMorse(const QString& morse) const
{
    QStringList chars = morse.split(' ', Qt::SkipEmptyParts);
    QString result;
    for (const QString& m : chars) {
        if (m_morseTable.contains(m))
            result += m_morseTable[m];
    }
    return result;
}

/* ---- Encode ---- */

QString PolluxCode::encode(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString morse = toMorse(plaintext);
    QString result;

    for (int i = 0; i < morse.size(); ++i) {
        QChar sym = morse[i];
        if (m_key.contains(sym) && !m_key[sym].isEmpty()) {
            const auto& digits = m_key[sym];
            int idx = qrand() % digits.size();
            result += QString::number(digits[idx]);
        }
    }

    m_stats.totalOps++;
    m_stats.encodeOps++;
    m_stats.lastInputLength = plaintext.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationCompleted("encode", plaintext.size(), timer.elapsed());
    return result;
}

/* ---- Decode ---- */

QString PolluxCode::decode(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    // Build reverse mapping: digit -> symbol
    QMap<int, QChar> reverse;
    for (auto it = m_key.constBegin(); it != m_key.constEnd(); ++it)
        for (int d : it.value())
            reverse[d] = it.key();

    // Convert digits to morse symbols
    QString morse;
    for (int i = 0; i < ciphertext.size(); ++i) {
        int digit = ciphertext[i].digitValue();
        if (digit >= 0 && reverse.contains(digit)) {
            QChar sym = reverse[digit];
            morse += sym;
        }
    }

    // Convert morse to text
    QString result = fromMorse(morse);

    const_cast<PolluxCode*>(this)->m_stats.totalOps++;
    const_cast<PolluxCode*>(this)->m_stats.decodeOps++;
    const_cast<PolluxCode*>(this)->m_stats.lastInputLength = ciphertext.size();
    m_timeSum += timer.elapsed();
    const_cast<PolluxCode*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    const_cast<PolluxCode*>(this)->operationCompleted(
        "decode", ciphertext.size(), timer.elapsed());
    return result;
}

/* ---- Frequency analysis ---- */

QMap<QChar, double> PolluxCode::frequencyAnalysis(const QString& ciphertext) const
{
    QMap<int, int> digitCounts;
    int total = 0;
    for (int i = 0; i < ciphertext.size(); ++i) {
        int d = ciphertext[i].digitValue();
        if (d >= 0) { digitCounts[d]++; total++; }
    }

    // Aggregate by symbol category
    QMap<QChar, double> freq;
    if (total == 0) return freq;

    // Reverse map digits to symbols
    QMap<int, QChar> reverse;
    for (auto it = m_key.constBegin(); it != m_key.constEnd(); ++it)
        for (int d : it.value()) reverse[d] = it.key();

    for (auto it = digitCounts.constBegin(); it != digitCounts.constEnd(); ++it) {
        QChar sym = reverse.contains(it.key()) ? reverse[it.key()] : '?';
        freq[sym] += static_cast<double>(it.value()) / total;
    }
    return freq;
}

/* ---- Crack key from known pair ---- */

QMap<QChar, QVector<int>> PolluxCode::crackKey(const QString& plain,
                                                 const QString& cipher) const
{
    QMap<QChar, QVector<int>> recovered;
    QString morse = const_cast<PolluxCode*>(this)->toMorse(plain);

    // Map each morse symbol to its cipher digit
    int ci = 0;
    for (int i = 0; i < morse.size() && ci < cipher.size(); ++i) {
        QChar sym = morse[i];
        int digit = cipher[ci].digitValue();
        if (digit >= 0) {
            if (!recovered[sym].contains(digit))
                recovered[sym].append(digit);
        }
        ci++;
    }

    // Deduplicate and sort
    for (auto it = recovered.begin(); it != recovered.end(); ++it)
        std::sort(it.value().begin(), it.value().end());

    return recovered;
}

/* ---- Reset ---- */

void PolluxCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
