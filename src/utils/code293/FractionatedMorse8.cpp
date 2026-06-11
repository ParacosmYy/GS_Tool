/**
 * @file FractionatedMorse8.cpp
 * @brief FractionatedMorse8 实现
 *
 * 实现分组摩尔斯密码：三字母置换表与变长摩尔斯符号分组实现分层分组密码。
 */

#include "utils/code293/FractionatedMorse8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

const QString FractionatedMorse8::ALPHABET = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

/* ---- Construction / Destruction ---- */

FractionatedMorse8::FractionatedMorse8(QObject *parent)
    : QObject(parent)
{
    initMorseTable();
    // Default identity key
    setKey(ALPHABET);
}

FractionatedMorse8::~FractionatedMorse8() = default;

/* ---- Initialize Morse code table ---- */

void FractionatedMorse8::initMorseTable()
{
    // Standard International Morse code (letters only)
    static const QVector<QPair<QChar, QString>> morseCodes = {
        {'A', ".-"},   {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
        {'E', "."},    {'F', "..-."}, {'G', "--."},  {'H', "...."},
        {'I', ".."},   {'J', ".---"}, {'K', "-.-"},  {'L', ".-.."},
        {'M', "--"},   {'N', "-."},   {'O', "---"},  {'P', ".--."},
        {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
        {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"},
        {'Y', "-.--"}, {'Z', "--.."}
    };
    for (const auto& pair : morseCodes) {
        m_morseTable[pair.first] = pair.second;
        m_reverseMorse[pair.second] = pair.first;
    }
}

/* ---- Generate standard trigrams ---- */

QVector<QString> FractionatedMorse8::generateStandardTrigrams() const
{
    // Generate all 26 Morse trigrams from combinations of ., -, x
    // Using lexicographic order: ..., ..x, ..-, .x., .xx, .x-, .-., etc.
    // The 'x' represents letter separator (pause) within trigram
    QVector<QString> trigrams;
    const QString symbols = QStringLiteral(".-x");
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k) {
                if (trigrams.size() >= 26) break;
                trigrams.append(QString(symbols[i]) + symbols[j] + symbols[k]);
            }
    return trigrams;
}

/* ---- Build trigram mappings from key ---- */

void FractionatedMorse8::buildTrigramMappings()
{
    auto trigrams = generateStandardTrigrams();
    m_trigramToLetter.clear();
    m_letterToTrigram.clear();

    QString effectiveKey = m_key.toUpper();
    // Pad or trim key to 26 characters
    while (effectiveKey.length() < 26)
        effectiveKey += ALPHABET[effectiveKey.length()];
    effectiveKey = effectiveKey.left(26);

    for (int i = 0; i < qMin(26, trigrams.size()); ++i) {
        m_trigramToLetter[trigrams[i]] = effectiveKey[i];
        m_letterToTrigram[effectiveKey[i]] = trigrams[i];
    }
}

/* ---- Configuration ---- */

void FractionatedMorse8::setKey(const QString& key)
{
    m_key = key.toUpper();
    buildTrigramMappings();
}

/* ---- Convert text to Morse ---- */

QString FractionatedMorse8::textToMorse(const QString& text) const
{
    QString morse;
    QString upper = text.toUpper();
    for (int i = 0; i < upper.length(); ++i) {
        QChar ch = upper[i];
        if (m_morseTable.contains(ch)) {
            if (!morse.isEmpty()) morse += 'x'; // Letter separator
            morse += m_morseTable[ch];
        }
        // Skip non-alpha characters
    }
    return morse;
}

/* ---- Convert Morse to text ---- */

QString FractionatedMorse8::morseToText(const QString& morse) const
{
    QString text;
    // Split by 'x' separator (representing letter boundary)
    QStringList symbols = morse.split('x', Qt::SkipEmptyParts);
    for (const QString& sym : symbols) {
        if (m_reverseMorse.contains(sym))
            text += m_reverseMorse[sym];
    }
    return text;
}

/* ---- Encrypt ---- */

FractionatedMorse8::CipherResult FractionatedMorse8::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    // Step 1: Convert plaintext to Morse symbols
    QString morseStr = textToMorse(plaintext);

    // Step 2: Pad to multiple of 3 for trigram grouping
    while (morseStr.length() % 3 != 0)
        morseStr += 'x';

    // Step 3: Group into trigrams and map to cipher letters
    QString cipherText;
    for (int i = 0; i + 2 < morseStr.length(); i += 3) {
        QString trigram = morseStr.mid(i, 3);
        if (m_trigramToLetter.contains(trigram))
            cipherText += m_trigramToLetter[trigram];
        else
            cipherText += 'X'; // Fallback
    }

    result.output = cipherText;
    result.inputLength = plaintext.length();
    result.outputLength = cipherText.length();

    double elapsed = timer.elapsed();
    m_stats.numEncrypts++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("encrypt"), result.inputLength,
                       result.outputLength, elapsed);
    return result;
}

/* ---- Decrypt ---- */

FractionatedMorse8::CipherResult FractionatedMorse8::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    CipherResult result;
    // Step 1: Map each cipher letter back to trigram
    QString morseStr;
    QString upper = ciphertext.toUpper();
    for (int i = 0; i < upper.length(); ++i) {
        QChar ch = upper[i];
        if (m_letterToTrigram.contains(ch))
            morseStr += m_letterToTrigram[ch];
    }

    // Step 2: Convert Morse string back to text
    // First, replace 'x' in trigrams with separator marker
    // The trigram 'x' acts as a pause/separator within Morse stream
    // We need to interpret the Morse symbols properly
    // Reconstruct by treating 'x' as a separator between letters
    QString plainText;

    // Parse the morse string: group consecutive . and - as symbols, x as separator
    QString currentSymbol;
    for (int i = 0; i < morseStr.length(); ++i) {
        QChar ch = morseStr[i];
        if (ch == 'x') {
            // Letter boundary
            if (!currentSymbol.isEmpty()) {
                if (m_reverseMorse.contains(currentSymbol))
                    plainText += m_reverseMorse[currentSymbol];
                currentSymbol.clear();
            }
        } else {
            currentSymbol += ch;
        }
    }
    // Flush last symbol
    if (!currentSymbol.isEmpty()) {
        if (m_reverseMorse.contains(currentSymbol))
            plainText += m_reverseMorse[currentSymbol];
    }

    result.output = plainText;
    result.inputLength = ciphertext.length();
    result.outputLength = plainText.length();

    double elapsed = timer.elapsed();
    m_stats.numDecrypts++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit operationDone(QStringLiteral("decrypt"), result.inputLength,
                       result.outputLength, elapsed);
    return result;
}

/* ---- Reset ---- */

void FractionatedMorse8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
