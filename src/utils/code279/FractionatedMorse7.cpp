/**
 * @file FractionatedMorse7.cpp
 * @brief FractionatedMorse7 实现
 *
 * 实现分裂摩尔斯密码：扩展三字符字母表与可逆摩尔斯-符号映射的双向编码。
 */

#include "utils/code279/FractionatedMorse7.h"

#include <QElapsedTimer>
#include <QRandomGenerator>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FractionatedMorse7::FractionatedMorse7(QObject *parent)
    : QObject(parent)
{
    initMorseTable();
    initTrigrahMapping();
}

FractionatedMorse7::~FractionatedMorse7() = default;

/* ---- Initialize standard Morse code table ---- */

void FractionatedMorse7::initMorseTable()
{
    // Standard A-Z Morse codes
    m_morseTable[QLatin1Char('A')] = QStringLiteral(".-");
    m_morseTable[QLatin1Char('B')] = QStringLiteral("-...");
    m_morseTable[QLatin1Char('C')] = QStringLiteral("-.-.");
    m_morseTable[QLatin1Char('D')] = QStringLiteral("-..");
    m_morseTable[QLatin1Char('E')] = QStringLiteral(".");
    m_morseTable[QLatin1Char('F')] = QStringLiteral("..-.");
    m_morseTable[QLatin1Char('G')] = QStringLiteral("--.");
    m_morseTable[QLatin1Char('H')] = QStringLiteral("....");
    m_morseTable[QLatin1Char('I')] = QStringLiteral("..");
    m_morseTable[QLatin1Char('J')] = QStringLiteral(".---");
    m_morseTable[QLatin1Char('K')] = QStringLiteral("-.-");
    m_morseTable[QLatin1Char('L')] = QStringLiteral(".-..");
    m_morseTable[QLatin1Char('M')] = QStringLiteral("--");
    m_morseTable[QLatin1Char('N')] = QStringLiteral("-.");
    m_morseTable[QLatin1Char('O')] = QStringLiteral("---");
    m_morseTable[QLatin1Char('P')] = QStringLiteral(".--.");
    m_morseTable[QLatin1Char('Q')] = QStringLiteral("--.-");
    m_morseTable[QLatin1Char('R')] = QStringLiteral(".-.");
    m_morseTable[QLatin1Char('S')] = QStringLiteral("...");
    m_morseTable[QLatin1Char('T')] = QStringLiteral("-");
    m_morseTable[QLatin1Char('U')] = QStringLiteral("..-");
    m_morseTable[QLatin1Char('V')] = QStringLiteral("...-");
    m_morseTable[QLatin1Char('W')] = QStringLiteral(".--");
    m_morseTable[QLatin1Char('X')] = QStringLiteral("-..-");
    m_morseTable[QLatin1Char('Y')] = QStringLiteral("-.--");
    m_morseTable[QLatin1Char('Z')] = QStringLiteral("--..");

    // Build reverse lookup
    for (auto it = m_morseTable.constBegin(); it != m_morseTable.constEnd(); ++it)
        m_reverseMorse[it.value()] = it.key();
}

/* ---- Initialize trigraph-to-symbol mapping ---- */

void FractionatedMorse7::initTrigrahMapping()
{
    // Generate all 3-symbol combinations of '.' '-' 'x' (separator)
    // 'x' = letter separator, mapped as filler to create 3-symbol trigraphs
    // Total: 26 combinations mapped to A-Z
    QVector<QString> trigrams;
    const QString symbols = QStringLiteral(".-x");
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k) {
                if (i == 2 && j == 2 && k == 2) continue; // Skip "xxx"
                trigrams.append(QString(symbols[i]) + symbols[j] + symbols[k]);
            }

    // Sort for deterministic mapping
    std::sort(trigrams.begin(), trigrams.end());

    // Map first 26 to A-Z
    for (int i = 0; i < TRIGRAPH_ALPHABET_SIZE && i < trigrams.size(); ++i) {
        QChar letter = QLatin1Char('A' + i);
        m_trigramToSymbol[trigrams[i]] = letter;
        m_symbolToTrigram[letter] = trigrams[i];
    }
}

/* ---- Pad Morse element to 3-symbol trigraph ---- */

QString FractionatedMorse7::padTrigraph(const QString& morse) const
{
    QString padded = morse;
    // Pad with 'x' (separator) to reach length 3
    while (padded.length() < 3)
        padded += QLatin1Char('x');
    return padded.left(3);
}

/* ---- Get Morse representation ---- */

QString FractionatedMorse7::toMorse(QChar ch) const
{
    QChar upper = ch.toUpper();
    return m_morseTable.value(upper, QString());
}

/* ---- Get character from Morse ---- */

QChar FractionatedMorse7::fromMorse(const QString& morse) const
{
    return m_reverseMorse.value(morse, QChar());
}

/* ---- Validate cipher text ---- */

bool FractionatedMorse7::isValidCipher(const QString& cipher) const
{
    for (const QChar& ch : cipher) {
        QChar upper = ch.toUpper();
        if (!m_symbolToTrigram.contains(upper) && !ch.isSpace())
            return false;
    }
    return true;
}

/* ---- Encode plaintext to fractionated Morse ---- */

FractionatedMorse7::MorseResult FractionatedMorse7::encode(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    MorseResult result;

    // Step 1: Convert each letter to Morse, join with 'x' separator
    QString morseStream;
    for (const QChar& ch : plaintext) {
        if (ch.isSpace()) {
            // Word separator: 'xx' (two letter separators)
            morseStream += QStringLiteral("xx");
        } else {
            QString code = toMorse(ch);
            if (!code.isEmpty()) {
                morseStream += code + QLatin1Char('x');
            }
        }
    }

    // Step 2: Pad stream length to multiple of 3
    while (morseStream.length() % 3 != 0)
        morseStream += QLatin1Char('x');

    // Step 3: Split into trigraphs and map to symbols
    QString cipherText;
    for (int i = 0; i + 2 < morseStream.length(); i += 3) {
        QString tri = morseStream.mid(i, 3);
        if (m_trigramToSymbol.contains(tri)) {
            cipherText += m_trigramToSymbol[tri];
        } else {
            // Fallback: unknown trigraph maps to 'Z'
            cipherText += QLatin1Char('Z');
        }
    }

    result.text = cipherText;
    result.success = true;
    result.inputLength = plaintext.length();
    result.outputLength = cipherText.length();

    const_cast<FractionatedMorse7*>(this)->m_stats.encodeCount++;
    const_cast<FractionatedMorse7*>(this)->m_stats.totalOps++;
    double elapsed = timer.elapsed();
    const_cast<FractionatedMorse7*>(this)->m_timeSum += elapsed;
    const_cast<FractionatedMorse7*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit encodeDone(result.inputLength, result.outputLength, elapsed);

    return result;
}

/* ---- Decode fractionated Morse to plaintext ---- */

FractionatedMorse7::MorseResult FractionatedMorse7::decode(const QString& cipher) const
{
    QElapsedTimer timer;
    timer.start();

    MorseResult result;

    // Step 1: Convert each cipher letter back to trigraph
    QString morseStream;
    for (const QChar& ch : cipher) {
        QChar upper = ch.toUpper();
        if (m_symbolToTrigram.contains(upper)) {
            morseStream += m_symbolToTrigram[upper];
        }
    }

    // Step 2: Parse Morse stream: split on 'x' separators
    // 'xx' = word boundary, single 'x' = letter boundary
    QString plaintext;
    QString currentMorse;
    int consecutiveX = 0;

    for (int i = 0; i < morseStream.length(); ++i) {
        QChar ch = morseStream[i];
        if (ch == QLatin1Char('x')) {
            // End of current Morse letter
            if (!currentMorse.isEmpty()) {
                QChar letter = fromMorse(currentMorse);
                if (!letter.isNull()) plaintext += letter;
                currentMorse.clear();
            }
            consecutiveX++;
            if (consecutiveX == 2) {
                plaintext += QLatin1Char(' ');  // Word boundary
                consecutiveX = 0;
            }
        } else {
            currentMorse += ch;
            consecutiveX = 0;
        }
    }
    // Flush remaining
    if (!currentMorse.isEmpty()) {
        QChar letter = fromMorse(currentMorse);
        if (!letter.isNull()) plaintext += letter;
    }

    result.text = plaintext;
    result.success = true;
    result.inputLength = cipher.length();
    result.outputLength = plaintext.length();

    const_cast<FractionatedMorse7*>(this)->m_stats.decodeCount++;
    const_cast<FractionatedMorse7*>(this)->m_stats.totalOps++;
    double elapsed = timer.elapsed();
    const_cast<FractionatedMorse7*>(this)->m_timeSum += elapsed;
    const_cast<FractionatedMorse7*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;
    emit decodeDone(result.inputLength, result.outputLength, elapsed);

    return result;
}

/* ---- Reset ---- */

void FractionatedMorse7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
