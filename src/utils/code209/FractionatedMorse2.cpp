/**
 * @file FractionatedMorse2.cpp
 * @brief FractionatedMorse2 实现
 *
 * 实现分组摩尔斯密码：三元频率分析、束搜索密钥恢复、统计密文攻击。
 */

#include "utils/code209/FractionatedMorse2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <random>

/* ---- Construction / Destruction ---- */

FractionatedMorse2::FractionatedMorse2(QObject *parent) : QObject(parent) {}
FractionatedMorse2::~FractionatedMorse2() = default;

void FractionatedMorse2::setBeamWidth(int width) { m_beamWidth = qMax(10, width); }

/* ---- Morse lookup table ---- */

QVector<QPair<QChar, QString>> FractionatedMorse2::morseTable()
{
    return {
        {'A', ".-"},    {'B', "-..."},  {'C', "-.-."},  {'D', "-.."},
        {'E', "."},     {'F', "..-."},  {'G', "--."},   {'H', "...."},
        {'I', ".."},    {'J', ".---"},  {'K', "-.-"},   {'L', ".-.."},
        {'M', "--"},    {'N', "-."},    {'O', "---"},   {'P', ".--."},
        {'Q', "--.-"},  {'R', ".-."},   {'S', "..."},   {'T', "-"},
        {'U', "..-"},   {'V', "...-"},  {'W', ".--"},   {'X', "-..-"},
        {'Y', "-.--"},  {'Z', "--.."},  {'0', "-----"}, {'1', ".----"},
        {'2', "..---"}, {'3', "...--"}, {'4', "....-"}, {'5', "....."},
        {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."},
        {'.', ".-.-.-"}, {',', "--..--"}, {'?', "..--.."}
    };
}

/* ---- Text to Morse ---- */

QString FractionatedMorse2::textToMorse(const QString& text)
{
    QString result;
    auto table = morseTable();
    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i].toUpper();
        if (ch == ' ') { result += 'x'; continue; }
        for (auto& p : table) {
            if (p.first == ch) {
                result += p.second + 'x';
                break;
            }
        }
    }
    return result;
}

/* ---- Morse to text ---- */

QString FractionatedMorse2::morseToText(const QString& morse)
{
    QString result;
    auto table = morseTable();
    QStringList parts = morse.split('x', Qt::SkipEmptyParts);
    for (auto& code : parts) {
        bool found = false;
        for (auto& p : table) {
            if (p.second == code) { result += p.first; found = true; break; }
        }
        if (!found) result += '?';
    }
    return result;
}

/* ---- Morse to trigraphs ---- */

QVector<QString> FractionatedMorse2::morseToTrigraphs(const QString& morse)
{
    // Pad to multiple of 3
    QString padded = morse;
    while (padded.size() % 3 != 0) padded += 'x';
    QVector<QString> trigraphs;
    for (int i = 0; i < padded.size(); i += 3)
        trigraphs.append(padded.mid(i, 3));
    return trigraphs;
}

/* ---- Trigraph index ---- */

int FractionatedMorse2::trigraphIndex(const QString& trigraph)
{
    // Map 3-symbol Morse group to index 0-25
    // Symbols: '.' = 0, '-' = 1, 'x' = 2 (ternary encoding)
    int idx = 0;
    for (int i = 0; i < 3 && i < trigraph.size(); ++i) {
        int digit = 0;
        if (trigraph[i] == '-') digit = 1;
        else if (trigraph[i] == 'x') digit = 2;
        idx = idx * 3 + digit;
    }
    return idx % 26;
}

/* ---- English letter frequencies ---- */

QVector<double> FractionatedMorse2::englishFreqs()
{
    return {8.167, 1.492, 2.782, 4.253, 12.702, 2.228, 2.015, 6.094, 6.966,
            0.153, 0.772, 4.025, 2.406, 6.749, 7.507, 1.929, 0.095, 5.987,
            6.327, 9.056, 2.758, 0.978, 2.360, 0.150, 1.974, 0.074};
}

/* ---- English bigram log-probabilities ---- */

QVector<QVector<double>> FractionatedMorse2::englishBigramLogProbs()
{
    // Simplified 26x26 bigram log-probs (lower = more common)
    QVector<QVector<double>> bg(26, QVector<double>(26, -3.0));
    // Common English bigrams get higher scores
    auto freq = englishFreqs();
    for (int i = 0; i < 26; ++i)
        for (int j = 0; j < 26; ++j)
            bg[i][j] = qLn(freq[i] * freq[j] / 10000.0 + 1e-10);
    return bg;
}

/* ---- Encrypt ---- */

QString FractionatedMorse2::encrypt(const QString& plaintext, const QString& key) const
{
    QElapsedTimer timer;
    timer.start();

    QString morse = textToMorse(plaintext);
    auto trigraphs = morseToTrigraphs(morse);
    QString cipher;

    for (auto& tri : trigraphs) {
        int idx = trigraphIndex(tri);
        if (idx < key.size()) cipher += key[idx].toUpper();
        else cipher += 'A';
    }

    auto self = const_cast<FractionatedMorse2*>(this);
    self->m_stats.totalOps++;
    self->m_stats.inputLength = plaintext.size();
    self->m_stats.keyLength = key.size();
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return cipher;
}

/* ---- Decrypt ---- */

QString FractionatedMorse2::decrypt(const QString& ciphertext, const QString& key) const
{
    QElapsedTimer timer;
    timer.start();

    QString morse;
    for (int i = 0; i < ciphertext.size(); ++i) {
        int idx = key.indexOf(ciphertext[i].toUpper());
        if (idx < 0) idx = 0;
        // Decode index to ternary trigraph
        int d0 = idx / 9;
        int d1 = (idx % 9) / 3;
        int d2 = idx % 3;
        QString tri;
        tri += (d0 == 0) ? '.' : (d0 == 1) ? '-' : 'x';
        tri += (d1 == 0) ? '.' : (d1 == 1) ? '-' : 'x';
        tri += (d2 == 0) ? '.' : (d2 == 1) ? '-' : 'x';
        morse += tri;
    }

    auto self = const_cast<FractionatedMorse2*>(this);
    self->m_stats.totalOps++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return morseToText(morse);
}

/* ---- Frequency score ---- */

double FractionatedMorse2::frequencyScore(const QString& ciphertext,
                                           const QString& candidateKey) const
{
    QString plain = decrypt(ciphertext, candidateKey);
    return englishScore(plain);
}

/* ---- English score via bigram frequencies ---- */

double FractionatedMorse2::englishScore(const QString& text) const
{
    auto bg = englishBigramLogProbs();
    double score = 0.0;
    int count = 0;
    for (int i = 0; i + 1 < text.size(); ++i) {
        int a = text[i].toUpper().toLatin1() - 'A';
        int b = text[i + 1].toUpper().toLatin1() - 'A';
        if (a >= 0 && a < 26 && b >= 0 && b < 26) {
            score += bg[a][b];
            count++;
        }
    }
    return (count > 0) ? score / count : -100.0;
}

/* ---- Beam search key recovery ---- */

QString FractionatedMorse2::crackKey(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    // Start with identity key A-Z
    QString initKey = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // Beam: maintain top-B candidate keys
    QVector<QPair<double, QString>> beam;
    beam.append({englishScore(decrypt(ciphertext, initKey)), initKey});

    std::mt19937 rng(42);

    // Iterative improvement: swap two positions and keep best
    for (int iter = 0; iter < 50; ++iter) {
        QVector<QPair<double, QString>> candidates;

        for (auto& entry : beam) {
            candidates.append(entry);
            // Generate neighbors by swapping pairs
            for (int s = 0; s < 30; ++s) {
                QString mutated = entry.second;
                int i = rng() % 26;
                int j = rng() % 26;
                if (i == j) continue;
                std::swap(mutated[i], mutated[j]);
                double sc = englishScore(decrypt(ciphertext, mutated));
                candidates.append({sc, mutated});
            }
        }

        // Keep top beamWidth candidates
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });
        if (candidates.size() > m_beamWidth)
            candidates.resize(m_beamWidth);
        beam = candidates;
    }

    QString bestKey = beam.isEmpty() ? initKey : beam[0].second;
    double bestScore = beam.isEmpty() ? -100.0 : beam[0].first;

    auto self = const_cast<FractionatedMorse2*>(this);
    self->m_stats.totalOps++;
    self->m_stats.beamWidth = m_beamWidth;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    self->emit keyCracked(bestKey, bestScore, timer.elapsed());

    return bestKey;
}

/* ---- Auto-decrypt ---- */

QString FractionatedMorse2::autoDecrypt(const QString& ciphertext) const
{
    QString key = crackKey(ciphertext);
    return decrypt(ciphertext, key);
}

/* ---- Reset ---- */

void FractionatedMorse2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
