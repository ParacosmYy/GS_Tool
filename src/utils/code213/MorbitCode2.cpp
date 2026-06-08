/**
 * @file MorbitCode2.cpp
 * @brief MorbitCode2 实现
 *
 * 实现Morbit密码：摩尔斯数字映射、频率直方图分析、密钥枚举攻击。
 */

#include "utils/code213/MorbitCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MorbitCode2::MorbitCode2(QObject *parent) : QObject(parent)
{
    buildMorseTable();
}

MorbitCode2::~MorbitCode2() = default;

/* ---- Build Morse lookup table ---- */

void MorbitCode2::buildMorseTable()
{
    // Standard Morse code for A-Z
    m_morseTable = {
        QStringLiteral(".-"),    // A
        QStringLiteral("-..."),  // B
        QStringLiteral("-.-."),  // C
        QStringLiteral("-.."),   // D
        QStringLiteral("."),     // E
        QStringLiteral("..-."),  // F
        QStringLiteral("--."),   // G
        QStringLiteral("...."),  // H
        QStringLiteral(".."),    // I
        QStringLiteral(".---"),  // J
        QStringLiteral("-.-"),   // K
        QStringLiteral(".-.."),  // L
        QStringLiteral("--"),    // M
        QStringLiteral("-."),    // N
        QStringLiteral("---"),   // O
        QStringLiteral(".--."),  // P
        QStringLiteral("--.-"),  // Q
        QStringLiteral(".-."),   // R
        QStringLiteral("..."),   // S
        QStringLiteral("-"),     // T
        QStringLiteral("..-"),   // U
        QStringLiteral("...-"),  // V
        QStringLiteral(".--"),   // W
        QStringLiteral("-..-"),  // X
        QStringLiteral("-.--"),  // Y
        QStringLiteral("--.."),  // Z
        QStringLiteral("-----"),// 0
        QStringLiteral(".----"),// 1
        QStringLiteral("..---"),// 2
        QStringLiteral("...--"),// 3
        QStringLiteral("....-"),// 4
        QStringLiteral("....."),// 5
        QStringLiteral("-...."),// 6
        QStringLiteral("--..."),// 7
        QStringLiteral("---.."),// 8
        QStringLiteral("----.") // 9
    };
    m_charTable.clear();
    for (char c = 'A'; c <= 'Z'; ++c) m_charTable.append(QChar(c));
    for (char c = '0'; c <= '9'; ++c) m_charTable.append(QChar(c));
}

/* ---- Validate key ---- */

bool MorbitCode2::validateKey(const QString& key) const
{
    if (key.length() != 9) return false;
    QVector<bool> used(9, false);
    for (int i = 0; i < 9; ++i) {
        int d = key[i].toLatin1() - '1';
        if (d < 0 || d >= 9 || used[d]) return false;
        used[d] = true;
    }
    return true;
}

/* ---- Set key ---- */

void MorbitCode2::setKey(const QString& key)
{
    if (!validateKey(key)) return;
    m_key = key;
    // Build key map: position i maps to digit key[i]
    // The 9 Morbit symbols represent all 3x3 combinations of {., -, /}
    m_keyMap.resize(9);
    for (int i = 0; i < 9; ++i) {
        int digit = key[i].toLatin1() - '1';
        m_keyMap[digit] = i;
    }
    m_stats.keyLength = 9;
}

/* ---- Morse pair encoding ---- */

int MorbitCode2::morsePairToIndex(QChar a, QChar b) const
{
    // Map pairs of {dot=0, dash=1, x/stop=2} to 0-8
    int va = (a == '.') ? 0 : (a == '-') ? 1 : 2;
    int vb = (b == '.') ? 0 : (b == '-') ? 1 : 2;
    return va * 3 + vb;
}

QPair<QChar, QChar> MorbitCode2::indexToMorsePair(int idx) const
{
    static const QChar symbols[] = {'.', '-', '/'};
    int va = idx / 3;
    int vb = idx % 3;
    return {symbols[va], symbols[vb]};
}

/* ---- Text to Morse ---- */

QString MorbitCode2::textToMorse(const QString& text) const
{
    QString morse;
    QString upper = text.toUpper();
    for (int i = 0; i < upper.size(); ++i) {
        QChar c = upper[i];
        if (c == ' ') { morse += '/'; continue; }
        int idx = -1;
        for (int j = 0; j < m_charTable.size(); ++j) {
            if (m_charTable[j] == c) { idx = j; break; }
        }
        if (idx >= 0 && idx < m_morseTable.size()) {
            morse += m_morseTable[idx];
        }
        if (i < upper.size() - 1 && upper[i + 1] != ' ') morse += ' ';
    }
    return morse;
}

/* ---- Morse to text ---- */

QString MorbitCode2::morseToText(const QString& morse) const
{
    QString result;
    QStringList letters = morse.split('/', Qt::SkipEmptyParts);
    for (const QString& group : letters) {
        QStringList symbols = group.split(' ', Qt::SkipEmptyParts);
        for (const QString& sym : symbols) {
            int idx = m_morseTable.indexOf(sym);
            if (idx >= 0 && idx < m_charTable.size())
                result += m_charTable[idx];
        }
        result += ' ';
    }
    return result.trimmed();
}

/* ---- Encode ---- */

QString MorbitCode2::encode(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString morse = textToMorse(plaintext);
    QString cipher;
    // Pair up Morse symbols and map through key
    for (int i = 0; i < morse.size(); i += 2) {
        QChar a = morse[i];
        QChar b = (i + 1 < morse.size()) ? morse[i + 1] : '/';
        int idx = morsePairToIndex(a, b);
        // Find which key position maps to this index
        for (int k = 0; k < 9; ++k) {
            if (m_keyMap[k] == idx) {
                cipher += QChar('1' + k);
                break;
            }
        }
    }

    // Update stats (const method for API consistency)
    MorbitCode2* self = const_cast<MorbitCode2*>(this);
    self->m_stats.messageLength = plaintext.size();
    self->m_stats.totalOps++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = self->m_timeSum / self->m_stats.totalOps;
    emit self->operationCompleted(
        QStringLiteral("encode"), cipher.size(), timer.elapsed());

    return cipher;
}

/* ---- Decode ---- */

QString MorbitCode2::decode(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString morse;
    for (int i = 0; i < ciphertext.size(); ++i) {
        int digit = ciphertext[i].toLatin1() - '1';
        if (digit < 0 || digit >= 9) continue;
        int idx = m_keyMap[digit];
        auto pair = indexToMorsePair(idx);
        morse += pair.first;
        morse += pair.second;
    }

    QString result = morseToText(morse);

    MorbitCode2* self = const_cast<MorbitCode2*>(this);
    self->m_stats.totalOps++;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = self->m_timeSum / self->m_stats.totalOps;
    emit self->operationCompleted(
        QStringLiteral("decode"), result.size(), timer.elapsed());

    return result;
}

/* ---- Frequency histogram ---- */

QVector<MorbitCode2::HistogramEntry> MorbitCode2::frequencyHistogram(
    const QString& ciphertext) const
{
    QMap<QChar, int> counts;
    int total = 0;
    for (const QChar& c : ciphertext) {
        if (c >= '1' && c <= '9') {
            counts[c]++;
            total++;
        }
    }
    QVector<HistogramEntry> hist;
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        HistogramEntry e;
        e.symbol = it.key();
        e.count = it.value();
        e.frequency = (total > 0) ? static_cast<double>(it.value()) / total : 0.0;
        hist.append(e);
    }
    std::sort(hist.begin(), hist.end(),
              [](const HistogramEntry& a, const HistogramEntry& b) {
                  return a.count > b.count;
              });
    return hist;
}

/* ---- Score a key candidate ---- */

double MorbitCode2::scoreKeyCandidate(const QString& key,
                                       const QString& ciphertext) const
{
    // Temporarily set key and decode, then score based on letter frequency
    MorbitCode2 temp;
    temp.buildMorseTable();
    temp.setKey(key);
    QString decoded = temp.decode(ciphertext);

    // Score using English letter frequency correlation
    static const double engFreq[] = {
        8.167, 1.492, 2.782, 4.253, 12.702, 2.228, 2.015, 6.094,
        6.966, 0.153, 0.772, 4.025, 2.406, 6.749, 7.507, 1.929,
        0.095, 5.987, 6.327, 9.056, 2.758, 0.978, 2.360, 0.150,
        1.974, 0.074
    };

    QMap<QChar, int> counts;
    int total = 0;
    for (const QChar& c : decoded.toUpper()) {
        if (c >= 'A' && c <= 'Z') { counts[c]++; total++; }
    }
    if (total == 0) return 0.0;

    double score = 0.0;
    for (char c = 'A'; c <= 'Z'; ++c) {
        double observed = (counts.contains(QChar(c)))
            ? static_cast<double>(counts[QChar(c)]) / total * 100.0 : 0.0;
        double expected = engFreq[c - 'A'];
        double diff = observed - expected;
        score -= diff * diff;
    }
    return score;
}

/* ---- Enumerate key candidates ---- */

QVector<QString> MorbitCode2::enumerateKeys(const QString& ciphertext,
                                             int maxCandidates) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, QString>> scored;

    // Generate permutations of 123456789 (sample first maxCandidates)
    QString digits = QStringLiteral("123456789");
    int tried = 0;
    QVector<int> perm = {0,1,2,3,4,5,6,7,8};

    do {
        QString candidate;
        for (int i = 0; i < 9; ++i)
            candidate += digits[perm[i]];
        double s = scoreKeyCandidate(candidate, ciphertext);
        scored.append({s, candidate});
        tried++;
        if (tried % 1000 == 0)
            emit const_cast<MorbitCode2*>(this)->keyEnumerationProgress(
                tried, maxCandidates);
        if (tried >= maxCandidates) break;
    } while (std::next_permutation(perm.begin(), perm.end()));

    std::sort(scored.begin(), scored.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    QVector<QString> result;
    for (int i = 0; i < qMin(maxCandidates, scored.size()); ++i)
        result.append(scored[i].second);

    MorbitCode2* self = const_cast<MorbitCode2*>(this);
    self->m_stats.candidatesTried = tried;
    self->m_timeSum += timer.elapsed();
    self->m_stats.totalOps++;
    self->m_stats.avgProcessingTimeMs = self->m_timeSum / self->m_stats.totalOps;

    return result;
}

/* ---- Reset ---- */

void MorbitCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_key.clear();
    m_keyMap.clear();
}
