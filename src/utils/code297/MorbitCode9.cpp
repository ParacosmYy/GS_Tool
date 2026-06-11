/**
 * @file MorbitCode9.cpp
 * @brief MorbitCode9 实现
 *
 * 实现Morbit密码：自适应符号表重排与频率相关替换实现动态手动密码加密。
 */

#include "utils/code297/MorbitCode9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MorbitCode9::MorbitCode9(QObject *parent)
    : QObject(parent)
{
    // Default alphabet: A-Z
    m_alphabet = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    for (int i = 0; i < 26; ++i) {
        SymbolEntry e;
        e.symbol = m_alphabet[i];
        e.frequency = 0;
        e.normFrequency = 0.0;
        m_symbolTable.append(e);
    }
}

MorbitCode9::~MorbitCode9() = default;

/* ---- Configuration ---- */

void MorbitCode9::setKey(const QString& key)
{
    // Normalize key to digits 1-9
    m_key.clear();
    for (QChar c : key) {
        if (c.isDigit()) {
            int d = c.digitValue();
            if (d >= 1 && d <= 9) m_key.append(c);
        }
    }
    if (m_key.isEmpty()) m_key = QStringLiteral("123456789");
}

void MorbitCode9::setAlphabet(const QString& alphabet)
{
    m_alphabet = alphabet.toUpper();
    m_symbolTable.clear();
    for (int i = 0; i < m_alphabet.size(); ++i) {
        SymbolEntry e;
        e.symbol = m_alphabet[i];
        e.frequency = 0;
        e.normFrequency = 0.0;
        m_symbolTable.append(e);
    }
}

/* ---- Morse code lookup ---- */

QString MorbitCode9::toMorse(QChar ch) const
{
    // Standard Morse code for A-Z and 0-9
    static const QVector<QString> morseCodes = {
        QStringLiteral(".-"),   QStringLiteral("-..."), QStringLiteral("-.-."),
        QStringLiteral("-.."),  QStringLiteral("."),    QStringLiteral("..-."),
        QStringLiteral("--."),  QStringLiteral("...."), QStringLiteral(".."),
        QStringLiteral(".---"), QStringLiteral("-.-"),  QStringLiteral(".-.."),
        QStringLiteral("--"),   QStringLiteral("-."),   QStringLiteral("---"),
        QStringLiteral(".--."), QStringLiteral("--.-"), QStringLiteral(".-."),
        QStringLiteral("..."),  QStringLiteral("-"),    QStringLiteral("..-"),
        QStringLiteral("...-"), QStringLiteral(".--"),  QStringLiteral("-..-"),
        QStringLiteral("-.--"), QStringLiteral("--.."),
        QStringLiteral("-----"), QStringLiteral(".----"), QStringLiteral("..---"),
        QStringLiteral("...--"), QStringLiteral("....-"), QStringLiteral("....."),
        QStringLiteral("-...."), QStringLiteral("--..."), QStringLiteral("---.."),
        QStringLiteral("----.")
    };

    QChar upper = ch.toUpper();
    int idx = -1;
    if (upper >= 'A' && upper <= 'Z')
        idx = upper.unicode() - 'A';
    else if (upper >= '0' && upper <= '9')
        idx = 26 + (upper.unicode() - '0');

    if (idx >= 0 && idx < morseCodes.size())
        return morseCodes[idx];
    return QStringLiteral(".");
}

/* ---- Convert Morse pair to digit index via key ---- */

int MorbitCode9::morsePairToIndex(const QString& morsePair) const
{
    // Map morse element pairs to digit indices 0-8
    // 9 possible combinations from (dot/dash/space) pairs mapped through key
    static const int pairMap[3][3] = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8}
    };

    int a = 0, b = 0;
    if (morsePair.size() >= 1) {
        if (morsePair[0] == '.') a = 0;
        else if (morsePair[0] == '-') a = 1;
        else a = 2;
    }
    if (morsePair.size() >= 2) {
        if (morsePair[1] == '.') b = 0;
        else if (morsePair[1] == '-') b = 1;
        else b = 2;
    }

    int baseIdx = pairMap[a][b];
    if (!m_key.isEmpty()) {
        int keyDigit = m_key[baseIdx % m_key.size()].digitValue();
        return (baseIdx + keyDigit) % 9;
    }
    return baseIdx;
}

/* ---- Reorder symbol table by frequency ---- */

void MorbitCode9::adaptSymbolTable()
{
    int totalFreq = 0;
    for (auto& e : m_symbolTable)
        totalFreq += e.frequency;

    if (totalFreq == 0) return;

    for (auto& e : m_symbolTable)
        e.normFrequency = static_cast<double>(e.frequency) / totalFreq;

    // Sort descending by frequency (stable to preserve relative order)
    std::stable_sort(m_symbolTable.begin(), m_symbolTable.end(),
        [](const SymbolEntry& a, const SymbolEntry& b) {
            return a.frequency > b.frequency;
        });
}

/* ---- Frequency-dependent substitution ---- */

QChar MorbitCode9::substitute(QChar input, int keyOffset)
{
    int idx = m_alphabet.indexOf(input.toUpper());
    if (idx < 0) return input;

    // Shift based on key offset and frequency-weighted position
    int alphaSize = m_alphabet.size();
    double freqShift = 0.0;
    if (idx < m_symbolTable.size())
        freqShift = m_symbolTable[idx].normFrequency * alphaSize * 0.5;

    int shifted = (idx + keyOffset + static_cast<int>(qFloor(freqShift))) % alphaSize;
    if (shifted < 0) shifted += alphaSize;

    if (shifted < m_symbolTable.size())
        return m_symbolTable[shifted].symbol;
    return m_alphabet[shifted];
}

/* ---- Reverse substitution ---- */

QChar MorbitCode9::reverseSubstitute(QChar cipher, int keyOffset)
{
    int idx = -1;
    for (int i = 0; i < m_symbolTable.size(); ++i) {
        if (m_symbolTable[i].symbol == cipher.toUpper()) { idx = i; break; }
    }
    if (idx < 0) return cipher;

    int alphaSize = m_alphabet.size();
    double freqShift = 0.0;
    if (idx < m_symbolTable.size())
        freqShift = m_symbolTable[idx].normFrequency * alphaSize * 0.5;

    int orig = (idx - keyOffset - static_cast<int>(qFloor(freqShift))) % alphaSize;
    if (orig < 0) orig += alphaSize;
    return m_alphabet[orig];
}

/* ---- Encrypt ---- */

MorbitCode9::EncResult MorbitCode9::encrypt(const QString& plainText)
{
    QElapsedTimer timer;
    timer.start();

    EncResult result;
    result.inputLength = plainText.size();

    QString cipher;
    QVector<int> indices;
    int keyIdx = 0;

    for (QChar ch : plainText) {
        QChar upper = ch.toUpper();
        if (!m_alphabet.contains(upper)) {
            cipher.append(ch);
            indices.append(0);
            continue;
        }

        // Convert to Morse and extract digit-pair indices
        QString morse = toMorse(upper);
        int morseIdx = 0;
        while (morseIdx < morse.size()) {
            QString pair;
            pair += morse[morseIdx];
            if (morseIdx + 1 < morse.size())
                pair += morse[morseIdx + 1];
            morseIdx += 2;

            int idx = morsePairToIndex(pair);
            int keyOffset = m_key.isEmpty() ? 0
                : m_key[keyIdx % m_key.size()].digitValue();

            QChar sub = substitute(upper, keyOffset + idx);
            cipher.append(sub);
            indices.append(idx);
            keyIdx++;
        }

        // Update frequency tracking
        int alphaIdx = m_alphabet.indexOf(upper);
        if (alphaIdx >= 0 && alphaIdx < m_symbolTable.size())
            m_symbolTable[alphaIdx].frequency++;
    }

    // Adapt table after each encryption
    adaptSymbolTable();

    result.cipherText = cipher;
    result.keyIndices = indices;

    double elapsed = timer.elapsed();
    m_stats.totalEncryptions++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit encryptDone(result.inputLength, elapsed);
    return result;
}

/* ---- Decrypt ---- */

QString MorbitCode9::decrypt(const QString& cipherText)
{
    QElapsedTimer timer;
    timer.start();

    QString plain;
    int keyIdx = 0;

    for (QChar ch : cipherText) {
        QChar upper = ch.toUpper();
        if (!m_alphabet.contains(upper)) {
            plain.append(ch);
            continue;
        }

        int keyOffset = m_key.isEmpty() ? 0
            : m_key[keyIdx % m_key.size()].digitValue();

        QChar rev = reverseSubstitute(upper, keyOffset);
        plain.append(rev);
        keyIdx++;
    }

    double elapsed = timer.elapsed();
    m_stats.totalDecryptions++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit decryptDone(cipherText.size(), elapsed);
    return plain;
}

/* ---- Frequency table ---- */

QVector<MorbitCode9::SymbolEntry> MorbitCode9::frequencyTable() const
{
    return m_symbolTable;
}

/* ---- Reset ---- */

void MorbitCode9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    for (auto& e : m_symbolTable) {
        e.frequency = 0;
        e.normFrequency = 0.0;
    }
}
