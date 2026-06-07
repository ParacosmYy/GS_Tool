/**
 * @file MorbitCode.cpp
 * @brief MorbitCode 实现
 *
 * 实现Morbit密码：3x3莫尔斯网格映射、频率分析密钥恢复。
 */

#include "utils/code194/MorbitCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MorbitCode::MorbitCode(QObject *parent) : QObject(parent) {}
MorbitCode::~MorbitCode() = default;

/* ---- Morse table ---- */

QVector<QPair<QChar, QString>> MorbitCode::morseTable()
{
    return {
        {'A', ".-"},   {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
        {'E', "."},    {'F', "..-."},  {'G', "--."},  {'H', "...."},
        {'I', ".."},   {'J', ".---"},  {'K', "-.-"},  {'L', ".-.."},
        {'M', "--"},   {'N', "-."},    {'O', "---"},  {'P', ".--."},
        {'Q', "--.-"}, {'R', ".-."},   {'S', "..."},  {'T', "-"},
        {'U', "..-"},  {'V', "...-"},  {'W', ".--"},  {'X', "-..-"},
        {'Y', "-.--"}, {'Z', "--.."},  {'0', "-----"},{'1', ".----"},
        {'2', "..---"},{'3', "...--"}, {'4', "....-"},{'5', "....."},
        {'6', "-...."},{'7', "--..."}, {'8', "---.."},{'9', "----."},
    };
}

QVector<QPair<QString, QChar>> MorbitCode::reverseMorse()
{
    QVector<QPair<QString, QChar>> rev;
    for (const auto& p : morseTable())
        rev.append({p.second, p.first});
    return rev;
}

/* ---- Set grid key ---- */

void MorbitCode::setGridKey(const QString& key)
{
    if (key.length() != 9) return;
    // Validate: digits 1-9, each appearing once
    QVector<bool> seen(10, false);
    for (int i = 0; i < 9; ++i) {
        int d = key[i].digitValue();
        if (d < 1 || d > 9 || seen[d]) return;
        seen[d] = true;
    }
    m_gridKey = key;
}

/* ---- Morse conversion ---- */

QString MorbitCode::toMorse(const QString& text) const
{
    QString result;
    auto table = morseTable();
    for (const QChar& ch : text.toUpper()) {
        if (ch == ' ') { result += ' '; continue; }
        for (const auto& entry : table) {
            if (entry.first == ch) { result += entry.second + ' '; break; }
        }
    }
    return result.trimmed();
}

QString MorbitCode::fromMorse(const QString& morse) const
{
    QString result;
    auto rev = reverseMorse();
    QStringList codes = morse.split(' ', Qt::SkipEmptyParts);
    for (const QString& code : codes) {
        bool found = false;
        for (const auto& entry : rev) {
            if (entry.first == code) { result += entry.second; found = true; break; }
        }
        if (!found) result += '?';
    }
    return result;
}

/* ---- Morse to digit pairs ---- */

QVector<QPair<int, int>> MorbitCode::morseToDigitPairs(const QString& morse) const
{
    // Map Morse symbols to 3x3 grid cells
    // Grid: rows=dit count position, cols=dah count position
    // . = row1, - = row2/row3; dit=col1, dah=col2/col3
    // Simplified: pair index in key for each morse symbol pair
    QVector<QPair<int, int>> pairs;

    // Convert morse string to symbol pairs (two symbols at a time)
    QString symbols;
    for (const QChar& ch : morse) {
        if (ch == '.' || ch == '-') symbols += ch;
    }

    for (int i = 0; i + 1 < symbols.size(); i += 2) {
        // Map symbol pair to 3x3 grid position
        int row = (symbols[i] == '.') ? 0 : ((symbols[i] == '-') ? 1 : 2);
        int col = (symbols[i+1] == '.') ? 0 : ((symbols[i+1] == '-') ? 1 : 2);
        pairs.append({row, col});
    }
    return pairs;
}

/* ---- Encode ---- */

QString MorbitCode::encode(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString morse = toMorse(plaintext);

    // Replace each morse symbol with grid digit
    QString result;
    for (int i = 0; i < morse.size(); ++i) {
        if (morse[i] == ' ') { result += ' '; continue; }
        // Map symbol to grid position
        // Pair consecutive symbols
        if (i + 1 < morse.size() && morse[i+1] != ' ') {
            int row = (morse[i] == '.') ? 0 : 1;
            int col = (morse[i+1] == '.') ? 0 : 1;
            // Extend to 3x3: use third row/col for some patterns
            row = row * 1; col = col * 1;
            int gridPos = row * 3 + col;
            if (gridPos < m_gridKey.size())
                result += m_gridKey[gridPos];
            i++; // skip next symbol (already consumed)
        } else {
            // Single symbol: use diagonal
            int pos = (morse[i] == '.') ? 0 : 4;
            if (pos < m_gridKey.size())
                result += m_gridKey[pos];
        }
    }

    const_cast<MorbitCode*>(this)->m_stats.totalEncodes++;
    const_cast<MorbitCode*>(this)->m_stats.lastInputLength = plaintext.size();
    m_timeSum += timer.elapsed();
    const_cast<MorbitCode*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    const_cast<MorbitCode*>(this)->encodeCompleted(plaintext.size(), timer.elapsed());
    return result;
}

/* ---- Decode ---- */

QString MorbitCode::decode(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    // Build reverse grid: digit -> symbol pair
    QString morseResult;
    for (int i = 0; i < ciphertext.size(); ++i) {
        if (ciphertext[i] == ' ') { morseResult += ' '; continue; }
        int digit = ciphertext[i].digitValue();
        if (digit < 1 || digit > 9) continue;
        // Find position in key
        int pos = m_gridKey.indexOf(QChar('0' + digit));
        if (pos < 0) continue;
        int row = pos / 3;
        int col = pos % 3;
        morseResult += (row <= 1) ? '.' : '-';
        morseResult += (col <= 1) ? '.' : '-';
    }

    QString result = fromMorse(morseResult);

    const_cast<MorbitCode*>(this)->m_stats.totalDecodes++;
    const_cast<MorbitCode*>(this)->m_stats.lastInputLength = ciphertext.size();
    m_timeSum += timer.elapsed();
    const_cast<MorbitCode*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    const_cast<MorbitCode*>(this)->decodeCompleted(ciphertext.size(), timer.elapsed());
    return result;
}

/* ---- Score key candidate ---- */

double MorbitCode::scoreKey(const QString& ciphertext, const QString& keyCandidate) const
{
    // English letter frequencies
    static const QVector<QPair<QChar, double>> engFreq = {
        {'E', 12.7}, {'T', 9.1}, {'A', 8.2}, {'O', 7.5}, {'I', 7.0},
        {'N', 6.7},  {'S', 6.3}, {'H', 6.1}, {'R', 6.0}, {'D', 4.3},
        {'L', 4.0},  {'C', 2.8}, {'U', 2.8}, {'M', 2.4}, {'W', 2.4},
        {'F', 2.2},  {'G', 2.0}, {'Y', 2.0}, {'P', 1.9}, {'B', 1.5},
        {'V', 1.0},  {'K', 0.8}, {'J', 0.2}, {'X', 0.2}, {'Q', 0.1},
        {'Z', 0.1}
    };

    // Temporarily use this key
    QString oldKey = m_gridKey;
    const_cast<MorbitCode*>(this)->m_gridKey = keyCandidate;
    QString decoded = decode(ciphertext);
    const_cast<MorbitCode*>(this)->m_gridKey = oldKey;

    // Score against English frequency
    QVector<int> counts(26, 0);
    int total = 0;
    for (const QChar& ch : decoded.toUpper()) {
        if (ch >= 'A' && ch <= 'Z') { counts[ch.toLatin1() - 'A']++; total++; }
    }
    if (total == 0) return 0.0;

    double score = 0.0;
    for (int i = 0; i < 26; ++i) {
        double observed = counts[i] / static_cast<double>(total) * 100.0;
        double expected = engFreq[i].second;
        score -= qAbs(observed - expected);
    }
    return score;
}

/* ---- Crack key ---- */

QVector<QPair<QString, double>> MorbitCode::crackKey(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QString, double>> candidates;

    // Try common key permutations (limited search for top candidates)
    QVector<int> perm = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    // Sample some permutations: try rotations and common patterns
    for (int shift = 0; shift < 9; ++shift) {
        QString key;
        for (int i = 0; i < 9; ++i)
            key += QChar('0' + perm[(i + shift) % 9]);

        double s = scoreKey(ciphertext, key);
        candidates.append({key, s});
    }

    // Reverse key
    QString revKey;
    for (int i = 8; i >= 0; --i) revKey += QChar('0' + perm[i]);
    candidates.append({revKey, scoreKey(ciphertext, revKey)});

    // Sort by score descending
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // Keep top 10
    if (candidates.size() > 10) candidates.resize(10);

    m_timeSum += timer.elapsed();
    return candidates;
}

/* ---- Reset ---- */

void MorbitCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
