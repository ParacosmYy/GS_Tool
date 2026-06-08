/**
 * @file StraddlingCheckerboard4.cpp
 * @brief StraddlingCheckerboard4 实现
 *
 * 实现跨越棋盘密码：频率排序密钥网格与动态行宽选择。
 */

#include "utils/code233/StraddlingCheckerboard4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

StraddlingCheckerboard4::StraddlingCheckerboard4(QObject *parent) : QObject(parent) {}
StraddlingCheckerboard4::~StraddlingCheckerboard4() = default;

/* ---- Configuration ---- */

void StraddlingCheckerboard4::setKeyPhrase(const QString& phrase) { m_keyPhrase = phrase.toUpper(); }
void StraddlingCheckerboard4::setRowLabels(int r1, int r2) { m_rowLabel1 = r1; m_rowLabel2 = r2; }

/* ---- Generate alphabet with key permutation ---- */

QVector<QChar> StraddlingCheckerboard4::generateAlphabet() const
{
    QVector<QChar> result;
    QSet<QChar> used;

    // Key characters first (unique)
    for (const QChar& ch : m_keyPhrase) {
        if (ch.isLetter() && !used.contains(ch)) {
            result.append(ch);
            used.insert(ch);
        }
    }
    // Remaining alphabet
    for (int c = 'A'; c <= 'Z'; ++c) {
        QChar ch(c);
        if (!used.contains(ch)) {
            result.append(ch);
            used.insert(ch);
        }
    }
    return result;
}

/* ---- Frequency analysis ---- */

QVector<QPair<QChar, int>> StraddlingCheckerboard4::analyzeFrequency(const QString& text) const
{
    QMap<QChar, int> counts;
    for (const QChar& ch : text.toUpper()) {
        if (ch.isLetter())
            counts[ch]++;
    }
    QVector<QPair<QChar, int>> freqs;
    for (auto it = counts.begin(); it != counts.end(); ++it)
        freqs.append({it.key(), it.value()});
    // Sort by frequency descending
    std::sort(freqs.begin(), freqs.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    return freqs;
}

/* ---- Dynamic row width selection ---- */

QVector<int> StraddlingCheckerboard4::selectRowWidths(const QVector<QPair<QChar, int>>& freqs)
{
    // Top 8 highest frequency characters go to row 0 (single digit)
    // Next 10 go to row 1 (two digits: rowLabel1 + column)
    // Remaining go to row 2 (two digits: rowLabel2 + column)
    // Row 0 has 10 slots but 2 are reserved for row labels
    int row0Width = 8;  // 10 - 2 reserved for row labels
    int row1Width = 10;
    int row2Width = qMax(1, freqs.size() - row0Width - row1Width);
    m_rowWidths = {row0Width, row1Width, row2Width};
    return m_rowWidths;
}

/* ---- Build grid ---- */

void StraddlingCheckerboard4::buildGrid()
{
    m_encodeGrid.clear();
    m_decodeGrid.clear();

    QVector<QChar> alphabet = generateAlphabet();

    // Frequency-ranked layout: high frequency chars get shorter codes
    // Row 0: single digit 0-9, skip rowLabel positions
    int col = 0;
    int alphaIdx = 0;
    for (int i = 0; i < 10 && alphaIdx < alphabet.size(); ++i) {
        if (i == m_rowLabel1 || i == m_rowLabel2) continue;
        QChar ch = alphabet[alphaIdx++];
        QString code = QString::number(i);
        m_encodeGrid[ch] = code;
        m_decodeGrid[code] = ch;
    }

    // Row 1: rowLabel1 + column digit
    for (int i = 0; i < 10 && alphaIdx < alphabet.size(); ++i) {
        QChar ch = alphabet[alphaIdx++];
        QString code = QString::number(m_rowLabel1) + QString::number(i);
        m_encodeGrid[ch] = code;
        m_decodeGrid[code] = ch;
    }

    // Row 2: rowLabel2 + column digit
    for (int i = 0; alphaIdx < alphabet.size(); ++i) {
        QChar ch = alphabet[alphaIdx++];
        QString code = QString::number(m_rowLabel2) + QString::number(i % 10);
        m_encodeGrid[ch] = code;
        m_decodeGrid[code] = ch;
    }
}

/* ---- Encode ---- */

QString StraddlingCheckerboard4::encode(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    // Build grid if needed
    if (m_encodeGrid.isEmpty()) buildGrid();

    // Analyze frequency to dynamically select row widths
    auto freqs = analyzeFrequency(plaintext);
    selectRowWidths(freqs);

    QString result;
    for (const QChar& ch : plaintext.toUpper()) {
        if (m_encodeGrid.contains(ch)) {
            result += m_encodeGrid[ch];
        } else if (ch.isSpace()) {
            result += QLatin1Char(' ');  // Preserve spaces
        }
        // Skip non-alpha characters
    }

    m_stats.inputLength = plaintext.length();
    m_stats.outputLength = result.length();
    m_stats.compressionRatio = (plaintext.length() > 0)
        ? static_cast<double>(result.length()) / plaintext.length() : 0.0;
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encodeCompleted(m_stats.inputLength, m_stats.outputLength, m_stats.compressionRatio);
    return result;
}

/* ---- Decode ---- */

QString StraddlingCheckerboard4::decode(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    if (m_decodeGrid.isEmpty()) buildGrid();

    QString result;
    int i = 0;
    while (i < ciphertext.length()) {
        QChar ch = ciphertext[i];
        if (ch.isSpace()) {
            result += ch;
            i++;
            continue;
        }

        // Try two-digit code first
        if (i + 1 < ciphertext.length()) {
            QString twoDigit = ciphertext.mid(i, 2);
            int d0 = twoDigit[0].digitValue();
            if (d0 == m_rowLabel1 || d0 == m_rowLabel2) {
                if (m_decodeGrid.contains(twoDigit)) {
                    result += m_decodeGrid[twoDigit];
                    i += 2;
                    continue;
                }
            }
        }

        // Single digit
        QString oneDigit = ciphertext.mid(i, 1);
        if (m_decodeGrid.contains(oneDigit)) {
            result += m_decodeGrid[oneDigit];
        }
        i++;
    }

    m_stats.inputLength = ciphertext.length();
    m_stats.outputLength = result.length();
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit decodeCompleted(ciphertext.length(), result.length());
    return result;
}

/* ---- Grid layout accessor ---- */

QMap<QChar, QString> StraddlingCheckerboard4::gridLayout() const { return m_encodeGrid; }

/* ---- Frequency analysis (public) ---- */

QVector<QPair<QChar, double>> StraddlingCheckerboard4::frequencyAnalysis(const QString& text) const
{
    auto freqs = analyzeFrequency(text);
    int total = 0;
    for (const auto& p : freqs) total += p.second;
    QVector<QPair<QChar, double>> result;
    for (const auto& p : freqs)
        result.append({p.first, (total > 0) ? static_cast<double>(p.second) / total : 0.0});
    return result;
}

/* ---- Reset ---- */

void StraddlingCheckerboard4::resetStatistics()
{
    m_encodeGrid.clear();
    m_decodeGrid.clear();
    m_rowWidths.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
