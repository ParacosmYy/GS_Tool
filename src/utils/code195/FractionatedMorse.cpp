/**
 * @file FractionatedMorse.cpp
 * @brief FractionatedMorse 实现
 *
 * 实现分体摩尔斯密码：摩尔斯编码、三字母列置换、统计分析。
 */

#include "utils/code195/FractionatedMorse.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FractionatedMorse::FractionatedMorse(QObject *parent) : QObject(parent)
{
    buildMorseTable();
    buildTrigraphTable();
}

FractionatedMorse::~FractionatedMorse() = default;

/* ---- Build Morse table ---- */

void FractionatedMorse::buildMorseTable()
{
    // Standard Morse code for A-Z
    static const char* letters[] = {
        ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
        "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
        "..-", "...-", ".--", "-..-", "-.--", "--.."
    };
    for (int i = 0; i < 26; ++i) {
        QChar c = QChar('A' + i);
        m_morseEncode[c] = QString(letters[i]);
        m_morseDecode[QString(letters[i])] = c;
    }
    // Digits 0-9
    static const char* digits[] = {
        "-----", ".----", "..---", "...--", "....-",
        ".....", "-....", "--...", "---..", "----."
    };
    for (int i = 0; i < 10; ++i) {
        QChar c = QChar('0' + i);
        m_morseEncode[c] = QString(digits[i]);
        m_morseDecode[QString(digits[i])] = c;
    }
}

/* ---- Build trigraph table ---- */

void FractionatedMorse::buildTrigraphTable()
{
    // 26 trigraphs from all 3-symbol combinations of . - x
    // where x = letter separator. 3^3 = 27, but we use 26 for A-Z.
    QString symbols = ".-x";
    int idx = 0;
    for (int i = 0; i < 3 && idx < 26; ++i)
        for (int j = 0; j < 3 && idx < 26; ++j)
            for (int k = 0; k < 3 && idx < 26; ++k) {
                m_trigraphTable.append(QString() + symbols[i] + symbols[j] + symbols[k]);
                idx++;
            }
}

/* ---- Derive column order ---- */

void FractionatedMorse::deriveColumnOrder()
{
    int n = m_keyword.size();
    if (n == 0) return;

    QVector<QPair<QChar, int>> indexed;
    for (int i = 0; i < n; ++i)
        indexed.append({m_keyword[i].toUpper(), i});

    std::sort(indexed.begin(), indexed.end(),
              [](const auto& a, const auto& b) {
                  return a.first < b.first || (a.first == b.first && a.second < b.second);
              });

    m_colOrder.resize(n);
    for (int i = 0; i < n; ++i)
        m_colOrder[indexed[i].second] = i;
}

/* ---- Configuration ---- */

void FractionatedMorse::setKeyword(const QString& keyword)
{
    m_keyword = keyword.toUpper();
    deriveColumnOrder();
}

/* ---- Convert to Morse string ---- */

QString FractionatedMorse::toMorseString(const QString& text) const
{
    QString result;
    QString upper = text.toUpper();
    for (int i = 0; i < upper.size(); ++i) {
        QChar c = upper[i];
        if (m_morseEncode.contains(c)) {
            result += m_morseEncode[c];
            if (i < upper.size() - 1) result += 'x'; // letter separator
        } else if (c == ' ') {
            result += 'x'; // word boundary = extra x
        }
    }
    return result;
}

/* ---- Convert from Morse string ---- */

QString FractionatedMorse::fromMorseString(const QString& morse) const
{
    QString result;
    QStringList parts = morse.split('x', Qt::SkipEmptyParts);
    for (const auto& p : parts) {
        if (m_morseDecode.contains(p))
            result += m_morseDecode[p];
        else
            result += '?';
    }
    return result;
}

/* ---- Morse to trigraphs ---- */

QString FractionatedMorse::morseToTrigraphs(const QString& morse) const
{
    // Pad to multiple of 3
    int padLen = (3 - morse.size() % 3) % 3;
    QString padded = morse;
    for (int i = 0; i < padLen; ++i) padded += 'x';

    QString result;
    for (int i = 0; i < padded.size(); i += 3) {
        QString tri = padded.mid(i, 3);
        // Map trigraph to letter A-Z
        for (int t = 0; t < qMin(26, m_trigraphTable.size()); ++t) {
            if (m_trigraphTable[t] == tri) {
                result += QChar('A' + t);
                break;
            }
        }
    }
    return result;
}

/* ---- Trigraphs to Morse ---- */

QString FractionatedMorse::trigraphsToMorse(const QString& trigraphs) const
{
    QString result;
    for (const QChar& c : trigraphs) {
        int idx = c.toUpper().toLatin1() - 'A';
        if (idx >= 0 && idx < m_trigraphTable.size())
            result += m_trigraphTable[idx];
    }
    return result;
}

/* ---- Columnar encrypt ---- */

QString FractionatedMorse::columnarEncrypt(const QString& text) const
{
    int n = m_colOrder.size();
    if (n == 0 || text.isEmpty()) return text;

    int rows = qCeil(static_cast<double>(text.size()) / n);
    QVector<QString> cols(n);

    for (int i = 0; i < text.size(); ++i)
        cols[i % n] += text[i];

    // Read columns in order
    QString result;
    for (int order = 0; order < n; ++order) {
        for (int c = 0; c < n; ++c) {
            if (m_colOrder[c] == order) {
                result += cols[c];
                break;
            }
        }
    }
    return result;
}

/* ---- Columnar decrypt ---- */

QString FractionatedMorse::columnarDecrypt(const QString& text) const
{
    int n = m_colOrder.size();
    if (n == 0 || text.isEmpty()) return text;

    int rows = qCeil(static_cast<double>(text.size()) / n);
    int fullCols = text.size() % n;
    if (fullCols == 0) fullCols = n;

    // Determine column lengths in keyword order
    QVector<int> colLens(n);
    for (int i = 0; i < n; ++i)
        colLens[i] = (i < fullCols) ? rows : rows - 1;

    // Read columns back from ciphertext in keyword order
    QVector<QString> cols(n);
    int pos = 0;
    for (int order = 0; order < n; ++order) {
        for (int c = 0; c < n; ++c) {
            if (m_colOrder[c] == order) {
                cols[c] = text.mid(pos, colLens[c]);
                pos += colLens[c];
                break;
            }
        }
    }

    // Read row by row
    QString result;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < n; ++c)
            if (r < cols[c].size()) result += cols[c][r];

    return result;
}

/* ---- Encrypt ---- */

QString FractionatedMorse::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    QString morse = toMorseString(plaintext);
    QString trigraphs = morseToTrigraphs(morse);

    // Apply columnar transposition if keyword set
    QString result = m_keyword.isEmpty() ? trigraphs : columnarEncrypt(trigraphs);

    m_stats.totalEncryptions++;
    m_stats.lastInputLength = plaintext.size();
    m_stats.lastOutputLength = result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit encryptionCompleted(plaintext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Decrypt ---- */

QString FractionatedMorse::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    // Reverse columnar transposition
    QString trigraphs = m_keyword.isEmpty() ? ciphertext : columnarDecrypt(ciphertext);
    QString morse = trigraphsToMorse(trigraphs);
    QString result = fromMorseString(morse);

    m_stats.totalDecryptions++;
    m_stats.lastInputLength = ciphertext.size();
    m_stats.lastOutputLength = result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit decryptionCompleted(ciphertext.size(), result.size(), timer.elapsed());
    return result;
}

/* ---- Morse frequency analysis ---- */

QMap<QChar, double> FractionatedMorse::analyzeMorseFrequency(const QString& ciphertext) const
{
    QMap<QChar, double> freq;
    int total = 0;
    for (const QChar& c : ciphertext.toUpper()) {
        if (c >= 'A' && c <= 'Z') {
            freq[c]++;
            total++;
        }
    }
    if (total > 0)
        for (auto it = freq.begin(); it != freq.end(); ++it)
            it.value() = (it.value() / total) * 100.0;
    return freq;
}

/* ---- Crack ---- */

QString FractionatedMorse::crack(const QString& ciphertext) const
{
    // Simple frequency-based crack: try all 26 keyword offsets
    QString bestResult;
    double bestScore = std::numeric_limits<double>::max();

    // Expected English letter frequencies
    static const double engFreq[] = {8.2,1.5,2.8,4.3,12.7,2.2,2.0,6.1,7.0,0.15,0.77,4.0,2.4,
                                      6.7,7.5,1.9,0.095,6.0,6.3,9.1,2.8,0.98,2.4,0.15,2.0,0.074};

    for (int shift = 0; shift < 26; ++shift) {
        QString shifted;
        for (const QChar& c : ciphertext.toUpper()) {
            if (c >= 'A' && c <= 'Z') {
                int idx = (c.toLatin1() - 'A' - shift + 26) % 26;
                shifted += QChar('A' + idx);
            } else {
                shifted += c;
            }
        }

        auto freq = analyzeMorseFrequency(shifted);
        double score = 0.0;
        for (auto it = freq.begin(); it != freq.end(); ++it) {
            int idx = it.key().toLatin1() - 'A';
            if (idx >= 0 && idx < 26)
                score += qPow(it.value() - engFreq[idx], 2);
        }

        if (score < bestScore) {
            bestScore = score;
            bestResult = shifted;
        }
    }

    // Convert best trigraph attempt back to Morse then plaintext
    QString morse = trigraphsToMorse(bestResult);
    return fromMorseString(morse);
}

/* ---- Accessors ---- */

QMap<QChar, QString> FractionatedMorse::morseTable() const { return m_morseEncode; }

/* ---- Reset ---- */

void FractionatedMorse::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
