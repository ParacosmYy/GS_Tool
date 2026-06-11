/**
 * @file PolluxCode8.cpp
 * @brief PolluxCode8 实现
 *
 * 实现Pollux密码：可配置点划莫尔斯映射与变长分隔符的灵活电报加密。
 */

#include "utils/code288/PolluxCode8.h"

#include <QElapsedTimer>
#include <QRandomGenerator>

/* ---- Construction / Destruction ---- */

PolluxCode8::PolluxCode8(QObject *parent)
    : QObject(parent)
{
    initMorseTable();
    initDefaultMapping();
}

PolluxCode8::~PolluxCode8() = default;

/* ---- Morse table initialization ---- */

void PolluxCode8::initMorseTable()
{
    // Standard International Morse code
    static const QPair<QChar, const char*> entries[] = {
        {'A', ".-"},   {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
        {'E', "."},    {'F', "..-."}, {'G', "--."},  {'H', "...."},
        {'I', ".."},   {'J', ".---"}, {'K', "-.-"},  {'L', ".-.."},
        {'M', "--"},   {'N', "-."},   {'O', "---"},  {'P', ".--."},
        {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
        {'U', "..-"},  {'V', "...-"}, {'W', ".--"},  {'X', "-..-"},
        {'Y', "-.--"}, {'Z', "--.."}, {'0', "-----"},{'1', ".----"},
        {'2', "..---"},{'3', "...--"},{'4', "....-"},{'5', "....."},
        {'6', "-...."},{'7', "--..."},{'8', "---.."},{'9', "----."},
    };
    for (const auto& e : entries) {
        m_morseTable[e.first] = QString::fromLatin1(e.second);
        m_reverseMorse[QString::fromLatin1(e.second)] = e.first;
    }
}

/* ---- Default digit-to-Morse mapping ---- */

void PolluxCode8::initDefaultMapping()
{
    // Standard Pollux mapping: digits 0-9 map to dot/dash/separator
    // This is configurable — a common assignment:
    // Dot: 1, 3, 5, 9   Dash: 2, 4, 6   Separator: 0, 7, 8
    m_digitMap.clear();
    m_digitMap[0] = Separator;
    m_digitMap[1] = Dot;
    m_digitMap[2] = Dash;
    m_digitMap[3] = Dot;
    m_digitMap[4] = Dash;
    m_digitMap[5] = Dot;
    m_digitMap[6] = Dash;
    m_digitMap[7] = Separator;
    m_digitMap[8] = Separator;
    m_digitMap[9] = Dot;
    rebuildReverseMap();
}

/* ---- Rebuild reverse mapping ---- */

void PolluxCode8::rebuildReverseMap()
{
    m_reverseMap.clear();
    for (auto it = m_digitMap.constBegin(); it != m_digitMap.constEnd(); ++it)
        m_reverseMap[it.value()].append(it.key());
}

/* ---- Configuration ---- */

void PolluxCode8::setMapping(const QMap<int, MorseElement>& mapping)
{
    m_digitMap = mapping;
    rebuildReverseMap();
}

void PolluxCode8::setDelimiter(const QString& delim) { m_delimiter = delim; }
void PolluxCode8::setWordSeparator(const QString& sep) { m_wordSeparator = sep; }

/* ---- Convert text to Morse code ---- */

QString PolluxCode8::toMorse(const QString& text) const
{
    QString result;
    bool first = true;
    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i].toUpper();
        if (ch == ' ') {
            result += m_wordSeparator;
            first = true;
            continue;
        }
        if (!m_morseTable.contains(ch)) continue;
        if (!first) result += m_delimiter;
        result += m_morseTable[ch];
        first = false;
    }
    return result;
}

/* ---- Convert Morse code to text ---- */

QString PolluxCode8::fromMorse(const QString& morse) const
{
    QString result;
    // Split by word separator first
    QStringList words = morse.split(m_wordSeparator, Qt::SkipEmptyParts);

    // Handle case where there's no word separator — treat as single word
    if (words.isEmpty()) {
        QStringList letters = morse.split(m_delimiter, Qt::SkipEmptyParts);
        for (const QString& code : letters) {
            if (m_reverseMorse.contains(code))
                result += m_reverseMorse[code];
        }
        return result;
    }

    for (int w = 0; w < words.size(); ++w) {
        if (w > 0) result += ' ';
        QStringList letters = words[w].split(m_delimiter, Qt::SkipEmptyParts);
        for (const QString& code : letters) {
            if (m_reverseMorse.contains(code))
                result += m_reverseMorse[code];
        }
    }
    return result;
}

/* ---- Encrypt: text -> Morse -> Pollux digit substitution ---- */

QString PolluxCode8::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Convert to Morse
    QString morse = toMorse(plaintext);

    // Step 2: Map each Morse element to a random digit
    QString cipher;
    cipher.reserve(morse.size());
    QRandomGenerator* rng = QRandomGenerator::global();

    for (int i = 0; i < morse.size(); ++i) {
        QChar ch = morse[i];
        MorseElement elem;
        if (ch == '.')
            elem = Dot;
        else if (ch == '-')
            elem = Dash;
        else
            elem = Separator;

        const QVector<int>& candidates = m_reverseMap[elem];
        if (candidates.isEmpty()) {
            cipher += '0'; // Fallback
        } else {
            cipher += QString::number(candidates[rng->bounded(candidates.size())]);
        }
    }

    double elapsed = timer.elapsed();
    const_cast<PolluxCode8*>(this)->m_stats.lastInputLen = plaintext.size();
    const_cast<PolluxCode8*>(this)->m_stats.lastCipherLen = cipher.size();
    const_cast<PolluxCode8*>(this)->m_stats.totalOps++;
    const_cast<PolluxCode8*>(this)->m_timeSum += elapsed;
    const_cast<PolluxCode8*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    const_cast<PolluxCode8*>(this)->operationDone(
        QStringLiteral("encrypt"), plaintext.size(), cipher.size(), elapsed);

    return cipher;
}

/* ---- Decrypt: Pollux digits -> Morse -> text ---- */

QString PolluxCode8::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Convert digits back to Morse elements
    QString morse;
    morse.reserve(ciphertext.size());
    for (int i = 0; i < ciphertext.size(); ++i) {
        int digit = ciphertext[i].digitValue();
        if (digit < 0) continue;
        if (!m_digitMap.contains(digit)) continue;
        MorseElement elem = m_digitMap[digit];
        if (elem == Dot) morse += '.';
        else if (elem == Dash) morse += '-';
        else morse += m_delimiter;
    }

    // Step 2: Convert Morse to text
    QString result = fromMorse(morse);

    double elapsed = timer.elapsed();
    const_cast<PolluxCode8*>(this)->m_stats.lastInputLen = ciphertext.size();
    const_cast<PolluxCode8*>(this)->m_stats.lastCipherLen = result.size();
    const_cast<PolluxCode8*>(this)->m_stats.totalOps++;
    const_cast<PolluxCode8*>(this)->m_timeSum += elapsed;
    const_cast<PolluxCode8*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    const_cast<PolluxCode8*>(this)->operationDone(
        QStringLiteral("decrypt"), ciphertext.size(), result.size(), elapsed);

    return result;
}

/* ---- Reset ---- */

void PolluxCode8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
