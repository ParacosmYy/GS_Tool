/**
 * @file FractionatedMorse6.cpp
 * @brief FractionatedMorse6 实现
 *
 * 实现分式摩尔斯密码：三字符替换与变长摩尔斯-三字符映射分式编码。
 */

#include "utils/code265/FractionatedMorse6.h"

#include <QElapsedTimer>
#include <QtGlobal>

/* ---- Construction / Destruction ---- */

FractionatedMorse6::FractionatedMorse6(QObject *parent)
    : QObject(parent)
{
    buildMorseTable();
    setKey(QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
}

FractionatedMorse6::~FractionatedMorse6() = default;

/* ---- Morse code table ---- */

void FractionatedMorse6::buildMorseTable()
{
    // Standard International Morse code
    m_morseTable['A'] = ".-";
    m_morseTable['B'] = "-...";
    m_morseTable['C'] = "-.-.";
    m_morseTable['D'] = "-..";
    m_morseTable['E'] = ".";
    m_morseTable['F'] = "..-.";
    m_morseTable['G'] = "--.";
    m_morseTable['H'] = "....";
    m_morseTable['I'] = "..";
    m_morseTable['J'] = ".---";
    m_morseTable['K'] = "-.-";
    m_morseTable['L'] = ".-..";
    m_morseTable['M'] = "--";
    m_morseTable['N'] = "-.";
    m_morseTable['O'] = "---";
    m_morseTable['P'] = ".--.";
    m_morseTable['Q'] = "--.-";
    m_morseTable['R'] = ".-.";
    m_morseTable['S'] = "...";
    m_morseTable['T'] = "-";
    m_morseTable['U'] = "..-";
    m_morseTable['V'] = "...-";
    m_morseTable['W'] = ".--";
    m_morseTable['X'] = "-..-";
    m_morseTable['Y'] = "-.--";
    m_morseTable['Z'] = "--..";

    for (auto it = m_morseTable.begin(); it != m_morseTable.end(); ++it)
        m_reverseMorse[it.value()] = it.key();
}

/* ---- Trigram table from key ---- */

void FractionatedMorse6::buildTrigramTable()
{
    // Generate all 26 trigrams from {'.','-','x'} in fixed order
    // 'x' is the separator (added between Morse letters)
    static const QString symbols = ".-x";
    QStringList trigrams;

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
                trigrams.append(QString(symbols[i]) + symbols[j] + symbols[k]);

    // Map first 26 trigrams to key letters
    m_trigramTable.clear();
    m_reverseTrigram.clear();
    for (int i = 0; i < 26 && i < trigrams.size(); ++i) {
        m_trigramTable[trigrams[i]] = m_key[i];
        m_reverseTrigram[m_key[i]] = trigrams[i];
    }
}

/* ---- Set key ---- */

void FractionatedMorse6::setKey(const QString& key)
{
    m_key = key.toUpper();
    // Remove duplicates while preserving order
    QString unique;
    for (QChar c : m_key) {
        if (c.isLetter() && !unique.contains(c))
            unique.append(c);
    }
    // Pad with remaining alphabet letters if needed
    for (char c = 'A'; c <= 'Z'; ++c) {
        if (!unique.contains(c)) unique.append(c);
    }
    m_key = unique.left(26);
    buildTrigramTable();
}

/* ---- Text <-> Morse conversion ---- */

QString FractionatedMorse6::textToMorse(const QString& text) const
{
    QStringList parts;
    for (QChar ch : text.toUpper()) {
        if (m_morseTable.contains(ch))
            parts.append(m_morseTable[ch]);
        else if (ch == ' ')
            parts.append(QString("x"));  // Word separator
    }
    // Join letters with 'x' separator
    QString result;
    for (int i = 0; i < parts.size(); ++i) {
        result += parts[i];
        if (i < parts.size() - 1) result += "x";
    }
    return result;
}

QString FractionatedMorse6::morseToText(const QString& morse) const
{
    // Split on 'x' separators
    QStringList parts = morse.split('x', Qt::SkipEmptyParts);
    QString result;
    for (const QString& part : parts) {
        if (m_reverseMorse.contains(part))
            result += m_reverseMorse[part];
    }
    return result;
}

/* ---- Pad Morse to multiple of 3 ---- */

QString FractionatedMorse6::padMorse(const QString& morse) const
{
    QString padded = morse;
    while (padded.size() % 3 != 0)
        padded += 'x';  // Pad with separator
    return padded;
}

/* ---- Split into trigrams ---- */

QVector<QString> FractionatedMorse6::splitTrigrams(const QString& morse) const
{
    QVector<QString> trigrams;
    for (int i = 0; i + 2 < morse.size(); i += 3)
        trigrams.append(morse.mid(i, 3));
    return trigrams;
}

/* ---- Encode ---- */

QString FractionatedMorse6::encode(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString morse = textToMorse(plaintext);
    QString padded = padMorse(morse);
    auto trigrams = splitTrigrams(padded);

    QString result;
    for (const QString& tg : trigrams) {
        if (m_trigramTable.contains(tg))
            result += m_trigramTable[tg];
    }

    double elapsed = timer.elapsed();
    m_stats.inputLength = plaintext.size();
    m_stats.outputLength = result.size();
    m_stats.trigraphCount = trigrams.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherComputed(plaintext.size(), result.size(), trigrams.size(), elapsed);
    return result;
}

/* ---- Decode ---- */

QString FractionatedMorse6::decode(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    // Convert each cipher letter back to trigram
    QString morseStream;
    for (QChar ch : ciphertext.toUpper()) {
        if (m_reverseTrigram.contains(ch))
            morseStream += m_reverseTrigram[ch];
    }

    // Convert Morse stream back to text
    QString result = morseToText(morseStream);

    double elapsed = timer.elapsed();
    m_stats.inputLength = ciphertext.size();
    m_stats.outputLength = result.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit cipherComputed(ciphertext.size(), result.size(), 0, elapsed);
    return result;
}

/* ---- Reset ---- */

void FractionatedMorse6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
