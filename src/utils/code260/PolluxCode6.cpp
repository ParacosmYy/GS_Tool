/**
 * @file PolluxCode6.cpp
 * @brief PolluxCode6 实现
 *
 * 实现Pollux密码：扩展莫尔斯元素集与约束传播密钥空间归约。
 */

#include "utils/code260/PolluxCode6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

PolluxCode6::PolluxCode6(QObject *parent)
    : QObject(parent) {}
PolluxCode6::~PolluxCode6() = default;

/* ---- Configuration ---- */

void PolluxCode6::setKey(const QVector<KeyEntry>& key) { m_key = key; m_keyLength = key.size(); }

/* ---- Morse conversion table ---- */

QVector<PolluxCode6::MorseElement> PolluxCode6::charToMorse(QChar ch) const
{
    // Standard Morse code mapping for A-Z, 0-9
    static const QHash<QChar, QString> morseMap = {
        {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."}, {'E', "."},
        {'F', "..-."}, {'G', "--."}, {'H', "...."}, {'I', ".."}, {'J', ".---"},
        {'K', "-.-"}, {'L', ".-.."}, {'M', "--"}, {'N', "-."}, {'O', "---"},
        {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."}, {'S', "..."}, {'T', "-"},
        {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"}, {'Y', "-.--"},
        {'Z', "--.."}, {'0', "-----"}, {'1', ".----"}, {'2', "..---"},
        {'3', "...--"}, {'4', "....-"}, {'5', "....."}, {'6', "-...."},
        {'7', "--..."}, {'8', "---.."}, {'9', "----."}
    };

    QVector<MorseElement> result;
    QChar upper = ch.toUpper();
    if (!morseMap.contains(upper)) return result;

    QString code = morseMap[upper];
    for (int i = 0; i < code.size(); ++i) {
        if (code[i] == QLatin1Char('.'))
            result.append(MorseElement::Dot);
        else if (code[i] == QLatin1Char('-'))
            result.append(MorseElement::Dash);
    }
    return result;
}

QChar PolluxCode6::morseToChar(const QVector<MorseElement>& elements) const
{
    // Build reverse lookup from element sequence to character
    QString code;
    for (const auto& e : elements) {
        if (e == MorseElement::Dot) code += QLatin1Char('.');
        else if (e == MorseElement::Dash) code += QLatin1Char('-');
    }

    static const QHash<QString, QChar> reverseMap = {
        {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'}, {".", 'E'},
        {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'}, {"..", 'I'}, {".---", 'J'},
        {"-.-", 'K'}, {".-..", 'L'}, {"--", 'M'}, {"-.", 'N'}, {"---", 'O'},
        {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
        {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'}, {"-.--", 'Y'},
        {"--..", 'Z'}, {"-----", '0'}, {".----", '1'}, {"..---", '2'},
        {"...--", '3'}, {"....-", '4'}, {".....", '5'}, {"-....", '6'},
        {"--...", '7'}, {"---..", '8'}, {"----.", '9'}
    };

    return reverseMap.value(code, QLatin1Char('?'));
}

/* ---- Generate random key ---- */

QVector<PolluxCode6::KeyEntry> PolluxCode6::generateKey(int length) const
{
    QVector<KeyEntry> key;
    key.reserve(length);
    for (int i = 0; i < length; ++i) {
        KeyEntry entry;
        entry.digit = i;
        entry.element = static_cast<MorseElement>(i % 3);
        key.append(entry);
    }
    return key;
}

/* ---- Insert letter spaces into Morse stream ---- */

QVector<PolluxCode6::MorseElement> PolluxCode6::insertLetterSpaces(
    const QVector<MorseElement>& raw) const
{
    // Insert LetterSpace between each character group
    QVector<MorseElement> result;
    for (int i = 0; i < raw.size(); ++i) {
        result.append(raw[i]);
        // Insert letter space after each dash (end of character boundary heuristic)
        if (raw[i] == MorseElement::Dash && i + 1 < raw.size()
            && raw[i + 1] != MorseElement::LetterSpace) {
            result.append(MorseElement::LetterSpace);
        }
    }
    return result;
}

/* ---- Encrypt ---- */

QVector<int> PolluxCode6::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    if (m_key.isEmpty()) return {};

    // Convert plaintext to Morse element sequence with letter spaces
    QVector<MorseElement> morseStream;
    for (int i = 0; i < plaintext.size(); ++i) {
        QVector<MorseElement> charMorse = charToMorse(plaintext[i]);
        if (!charMorse.isEmpty()) {
            morseStream.append(charMorse);
            if (i + 1 < plaintext.size())
                morseStream.append(MorseElement::LetterSpace);
        }
    }

    // Map each Morse element to a digit from the key
    QVector<int> ciphertext;
    for (const auto& elem : morseStream) {
        // Find matching key entries for this element
        QVector<int> candidates;
        for (const auto& entry : m_key) {
            if (entry.element == elem) candidates.append(entry.digit);
        }
        if (!candidates.isEmpty()) {
            // Pick a candidate (deterministic: first match)
            ciphertext.append(candidates[0]);
        }
    }

    double elapsed = timer.elapsed();
    m_stats.numEncryptions++;
    m_stats.totalOps++;
    m_stats.keyLength = m_key.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encryptionCompleted(plaintext.size(), ciphertext.size(), elapsed);
    return ciphertext;
}

/* ---- Decrypt via constraint propagation ---- */

QString PolluxCode6::decrypt(const QVector<int>& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    if (ciphertext.isEmpty() || m_key.isEmpty()) return {};

    // Build element lookup from key
    QVector<MorseElement> elements(ciphertext.size());
    for (int i = 0; i < ciphertext.size(); ++i) {
        int digit = ciphertext[i];
        if (digit >= 0 && digit < m_key.size()) {
            elements[i] = m_key[digit].element;
        } else {
            elements[i] = MorseElement::LetterSpace;  // Unknown digit
        }
    }

    // Parse Morse stream: split by LetterSpace into character groups
    QString result;
    QVector<MorseElement> currentChar;
    for (const auto& elem : elements) {
        if (elem == MorseElement::LetterSpace) {
            if (!currentChar.isEmpty()) {
                result.append(morseToChar(currentChar));
                currentChar.clear();
            }
        } else {
            currentChar.append(elem);
        }
    }
    if (!currentChar.isEmpty())
        result.append(morseToChar(currentChar));

    double elapsed = timer.elapsed();
    m_stats.numDecryptions++;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decryptionCompleted(ciphertext.size(), result, elapsed);
    return result;
}

/* ---- Constraint propagation for key space reduction ---- */

QVector<QVector<PolluxCode6::KeyEntry>> PolluxCode6::reduceKeyspace(
    const QVector<int>& ciphertext, const QString& knownPlaintext) const
{
    QElapsedTimer timer;
    timer.start();

    auto constraints = buildConstraints(ciphertext, knownPlaintext);
    QVector<QVector<KeyEntry>> validKeys;

    // Generate all possible single-digit key entries
    QVector<MorseElement> elemTypes = {MorseElement::Dot, MorseElement::Dash,
                                        MorseElement::LetterSpace};

    // Try all possible key assignments (limited key length)
    int maxKeyLen = qMin(m_keyLength, 10);
    QVector<KeyEntry> trialKey(maxKeyLen);
    for (int digit = 0; digit < maxKeyLen; ++digit) {
        trialKey[digit].digit = digit;
        trialKey[digit].element = MorseElement::Dot;
    }

    // Iterate through all 3^maxKeyLen combinations (bounded)
    int totalCombos = 1;
    for (int i = 0; i < maxKeyLen; ++i) totalCombos *= 3;

    int originalSize = totalCombos;
    for (int combo = 0; combo < totalCombos; ++combo) {
        int tmp = combo;
        for (int d = 0; d < maxKeyLen; ++d) {
            trialKey[d].element = elemTypes[tmp % 3];
            tmp /= 3;
        }
        if (satisfiesConstraints(trialKey, constraints))
            validKeys.append(trialKey);
    }

    double elapsed = timer.elapsed();
    m_stats.keyspaceReductionPercent = (originalSize > 0)
        ? (100 * (originalSize - validKeys.size()) / originalSize) : 0;
    emit keyspaceReduced(originalSize, validKeys.size());
    return validKeys;
}

QVector<QVector<PolluxCode6::MorseElement>> PolluxCode6::buildConstraints(
    const QVector<int>& cipher, const QString& known) const
{
    // Build expected Morse elements from known plaintext
    QVector<QVector<MorseElement>> constraints;
    for (int i = 0; i < known.size(); ++i) {
        auto morse = charToMorse(known[i]);
        if (!morse.isEmpty()) {
            constraints.append(morse);
            if (i + 1 < known.size())
                constraints.append({MorseElement::LetterSpace});
        }
    }
    return constraints;
}

bool PolluxCode6::satisfiesConstraints(const QVector<KeyEntry>& key,
    const QVector<QVector<MorseElement>>& constraints) const
{
    // Check that key can produce the constraint elements
    for (const auto& group : constraints) {
        for (const auto& elem : group) {
            bool found = false;
            for (const auto& entry : key) {
                if (entry.element == elem) { found = true; break; }
            }
            if (!found) return false;
        }
    }
    return true;
}

/* ---- Reset ---- */

void PolluxCode6::resetStatistics()
{
    m_key.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
