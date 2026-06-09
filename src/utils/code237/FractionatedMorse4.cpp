/**
 * @file FractionatedMorse4.cpp
 * @brief FractionatedMorse4 实现
 *
 * 实现分组摩尔斯密码：三字母代换表与可配置码元时序。
 */

#include "utils/code237/FractionatedMorse4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FractionatedMorse4::FractionatedMorse4(QObject *parent) : QObject(parent)
{
    initMorseTable();
    buildSubstitutionTable();
}

FractionatedMorse4::~FractionatedMorse4() = default;

/* ---- Initialize Morse table ---- */

void FractionatedMorse4::initMorseTable()
{
    // Standard International Morse code
    m_charToMorse['A'] = ".-";     m_charToMorse['B'] = "-...";
    m_charToMorse['C'] = "-.-.";   m_charToMorse['D'] = "-..";
    m_charToMorse['E'] = ".";      m_charToMorse['F'] = "..-.";
    m_charToMorse['G'] = "--.";    m_charToMorse['H'] = "....";
    m_charToMorse['I'] = "..";     m_charToMorse['J'] = ".---";
    m_charToMorse['K'] = "-.-";    m_charToMorse['L'] = ".-..";
    m_charToMorse['M'] = "--";     m_charToMorse['N'] = "-.";
    m_charToMorse['O'] = "---";    m_charToMorse['P'] = ".--.";
    m_charToMorse['Q'] = "--.-";   m_charToMorse['R'] = ".-.";
    m_charToMorse['S'] = "...";    m_charToMorse['T'] = "-";
    m_charToMorse['U'] = "..-";    m_charToMorse['V'] = "...-";
    m_charToMorse['W'] = ".--";    m_charToMorse['X'] = "-..-";
    m_charToMorse['Y'] = "-.--";   m_charToMorse['Z'] = "--..";

    // Build reverse map
    for (auto it = m_charToMorse.begin(); it != m_charToMorse.end(); ++it)
        m_morseToChar[it.value()] = it.key();
}

/* ---- Build substitution table ---- */

void FractionatedMorse4::buildSubstitutionTable()
{
    // Generate alphabet: keyword (unique letters) + remaining in order
    QString alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString cipherAlpha;
    if (!m_keyword.isEmpty()) {
        QString kw = m_keyword.toUpper();
        for (int i = 0; i < kw.size(); ++i)
            if (cipherAlpha.indexOf(kw[i]) < 0 && kw[i] >= 'A' && kw[i] <= 'Z')
                cipherAlpha.append(kw[i]);
        for (int i = 0; i < alpha.size(); ++i)
            if (cipherAlpha.indexOf(alpha[i]) < 0)
                cipherAlpha.append(alpha[i]);
    } else {
        cipherAlpha = alpha;
    }

    // Generate all 26 trigrams from {., -, x} where x = separator
    // Trigram patterns: 3^2 = 9 + remaining 17 = 26 total
    QString elements = ".-x";
    QVector<QString> trigrams;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
                if (!(i == 2 && j == 2 && k == 2)) // exclude xxx
                    trigrams.append(QString("%1%2%3").arg(elements[i]).arg(elements[j]).arg(elements[k]));

    // Only need 26 trigrams for 26 letters
    trigrams.resize(26);

    m_trigramToLetter.clear();
    m_letterToTrigram.clear();
    for (int i = 0; i < 26 && i < trigrams.size(); ++i) {
        m_trigramToLetter[trigrams[i]] = cipherAlpha[i];
        m_letterToTrigram[cipherAlpha[i]] = trigrams[i];
    }
}

/* ---- Split into trigrams ---- */

QVector<QString> FractionatedMorse4::splitTrigrams(const QString& morseStream) const
{
    QVector<QString> result;
    int len = morseStream.size();
    for (int i = 0; i + 2 < len; i += 3)
        result.append(morseStream.mid(i, 3));
    // Handle remaining 1 or 2 characters: pad with 'x'
    if (len % 3 != 0) {
        QString last = morseStream.mid(len - len % 3);
        while (last.size() < 3) last += 'x';
        result.append(last);
    }
    return result;
}

/* ---- Configuration ---- */

void FractionatedMorse4::setTiming(const Timing& t) { m_timing = t; }

void FractionatedMorse4::setKeyword(const QString& keyword)
{
    m_keyword = keyword;
    buildSubstitutionTable();
}

/* ---- Text to Morse ---- */

QString FractionatedMorse4::textToMorse(const QString& text) const
{
    QString result;
    QString upper = text.toUpper();
    for (int i = 0; i < upper.size(); ++i) {
        QChar ch = upper[i];
        if (ch == ' ') {
            result += 'x'; // word separator as extra x
        } else if (m_charToMorse.contains(ch)) {
            result += m_charToMorse[ch];
            result += 'x'; // inter-character separator
        }
    }
    return result;
}

/* ---- Morse to text ---- */

QString FractionatedMorse4::morseToText(const QString& morse) const
{
    QString result;
    QString current;
    for (int i = 0; i < morse.size(); ++i) {
        QChar ch = morse[i];
        if (ch == 'x' || ch == ' ') {
            if (!current.isEmpty() && m_morseToChar.contains(current))
                result += m_morseToChar[current];
            current.clear();
        } else {
            current += ch;
        }
    }
    if (!current.isEmpty() && m_morseToChar.contains(current))
        result += m_morseToChar[current];
    return result;
}

/* ---- Encode ---- */

QString FractionatedMorse4::encode(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // Convert plaintext to Morse stream with 'x' separators
    QString morseStream = textToMorse(plaintext);
    // Remove trailing 'x' if present
    while (morseStream.endsWith('x') && morseStream.size() > 0)
        morseStream.chop(1);

    // Split into trigrams and substitute
    QVector<QString> trigrams = splitTrigrams(morseStream);
    QString ciphertext;
    for (const auto& tri : trigrams) {
        if (m_trigramToLetter.contains(tri))
            ciphertext += m_trigramToLetter[tri];
        else
            ciphertext += '?';
    }

    m_stats.numEncoded++;
    m_stats.totalCharacters += plaintext.size();
    m_stats.totalTrigrams += trigrams.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encodeCompleted(plaintext.size(), trigrams.size(), timer.elapsed());
    return ciphertext;
}

/* ---- Decode ---- */

QString FractionatedMorse4::decode(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Reverse substitution: each cipher letter -> trigram
    QString morseStream;
    QString upper = ciphertext.toUpper();
    for (int i = 0; i < upper.size(); ++i) {
        QChar ch = upper[i];
        if (m_letterToTrigram.contains(ch))
            morseStream += m_letterToTrigram[ch];
    }

    // Remove trailing x separators and convert back to text
    while (morseStream.endsWith('x'))
        morseStream.chop(1);
    morseStream += 'x'; // ensure final separator

    QString plaintext = morseToText(morseStream);

    m_stats.numDecoded++;
    m_stats.totalTrigrams += ciphertext.size();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decodeCompleted(plaintext.size(), ciphertext.size(), timer.elapsed());
    return plaintext;
}

/* ---- Accessors ---- */

QMap<QString, QChar> FractionatedMorse4::substitutionTable() const
{
    return m_trigramToLetter;
}

/* ---- Reset ---- */

void FractionatedMorse4::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
