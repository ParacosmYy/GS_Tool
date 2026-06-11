/**
 * @file FoursquareCode16.cpp
 * @brief FoursquareCode16 实现
 *
 * 实现四方密码：同音替换与Polybius坐标抖动实现频率隐藏增强表加密。
 */

#include "utils/code300/FoursquareCode16.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

FoursquareCode16::FoursquareCode16(QObject *parent)
    : QObject(parent)
{
    // Initialize with default config
    m_config = Config{};
    m_tlSquare = buildSquare(QString());
    m_brSquare = buildSquare(QString());
    m_trSquare = buildSquare(m_config.key1);
    m_blSquare = buildSquare(m_config.key2);
}

FoursquareCode16::~FoursquareCode16() = default;

/* ---- Configuration ---- */

void FoursquareCode16::setConfig(const Config& config)
{
    m_config = config;
    m_tlSquare = buildSquare(QString());    // Plain alphabet
    m_brSquare = buildSquare(QString());    // Plain alphabet
    m_trSquare = buildSquare(config.key1);
    m_blSquare = buildSquare(config.key2);
}

/* ---- Prepare key: remove duplicates, merge with alphabet ---- */

QString FoursquareCode16::prepareKey(const QString& key) const
{
    QString cleaned;
    for (QChar ch : key.toUpper()) {
        if (ch == QLatin1Char('J')) ch = QLatin1Char('I');
        if (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z') && !cleaned.contains(ch))
            cleaned.append(ch);
    }
    // Fill remaining alphabet
    for (QChar ch : m_config.alphabet) {
        if (!cleaned.contains(ch))
            cleaned.append(ch);
    }
    return cleaned.left(25);
}

/* ---- Build 5x5 Polybius square from key ---- */

QVector<QVector<QChar>> FoursquareCode16::buildSquare(const QString& key) const
{
    QString prepared = prepareKey(key);
    QVector<QVector<QChar>> square(5, QVector<QChar>(5));
    for (int i = 0; i < 25 && i < prepared.size(); ++i)
        square[i / 5][i % 5] = prepared[i];
    return square;
}

/* ---- Find character position in square ---- */

QPair<int, int> FoursquareCode16::findPosition(const QVector<QVector<QChar>>& square, QChar ch) const
{
    if (ch == QLatin1Char('J')) ch = QLatin1Char('I');
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            if (square[r][c] == ch)
                return {r, c};
    return {0, 0};   // Default if not found
}

/* ---- Preprocess: uppercase, J->I, pad ---- */

QString FoursquareCode16::preprocess(const QString& text) const
{
    QString result;
    for (QChar ch : text.toUpper()) {
        if (ch == QLatin1Char('J')) ch = QLatin1Char('I');
        if (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z'))
            result.append(ch);
    }
    // Pad with 'X' if odd length
    if (result.size() % 2 != 0)
        result.append(QLatin1Char('X'));
    return result;
}

/* ---- Build homophone table ---- */

void FoursquareCode16::buildHomophoneTable(const QString& referenceText)
{
    QMap<QChar, int> freq = frequencyAnalysis(referenceText);
    m_homophones.clear();

    // Sort characters by frequency (descending)
    QVector<QChar> sorted;
    for (auto it = freq.begin(); it != freq.end(); ++it)
        sorted.append(it.key());
    std::sort(sorted.begin(), sorted.end(), [&](QChar a, QChar b) {
        return freq[a] > freq[b];
    });

    // Assign multiple code points to high-frequency characters
    int code = 0;
    for (QChar ch : sorted) {
        int count = qMax(1, freq[ch] * 5 / (referenceText.size() + 1) + 1);
        count = qMin(count, 4);  // Max 4 homophones per character
        QVector<int> codes;
        for (int i = 0; i < count; ++i)
            codes.append(code++);
        m_homophones[ch] = codes;
    }
}

/* ---- Apply coordinate jitter ---- */

QPair<int, int> FoursquareCode16::applyJitter(int row, int col) const
{
    // Deterministic jitter based on position (reproducible)
    int jRow = static_cast<int>(qSin(row * 17 + col * 31) * m_config.jitterStrength) % 2;
    int jCol = static_cast<int>(qCos(row * 23 + col * 37) * m_config.jitterStrength) % 2;
    return {(row + jRow + 5) % 5, (col + jCol + 5) % 5};
}

/* ---- Remove jitter from coordinates ---- */

QPair<int, int> FoursquareCode16::removeJitter(int row, int col) const
{
    int jRow = static_cast<int>(qSin(row * 17 + col * 31) * m_config.jitterStrength) % 2;
    int jCol = static_cast<int>(qCos(row * 23 + col * 37) * m_config.jitterStrength) % 2;
    return {(row - jRow + 5) % 5, (col - jCol + 5) % 5};
}

/* ---- Encrypt ---- */

FoursquareCode16::EncryptResult FoursquareCode16::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    EncryptResult result;
    QString processed = preprocess(plaintext);
    int n = processed.size();
    if (n < 2) { result.elapsedMs = timer.elapsed(); return result; }

    // Build homophone table if enabled
    if (m_config.homophonicEnabled)
        buildHomophoneTable(processed);

    QString cipher;
    int digraphs = 0;

    for (int i = 0; i < n - 1; i += 2) {
        QChar a = processed[i];
        QChar b = processed[i + 1];

        // Find positions in top-left and bottom-right (plain squares)
        auto [r1, c1] = findPosition(m_tlSquare, a);
        auto [r2, c2] = findPosition(m_brSquare, b);

        // Apply jitter for frequency hiding
        if (m_config.jitterStrength > 0.0) {
            auto j1 = applyJitter(r1, c1);
            auto j2 = applyJitter(r2, c2);
            r1 = j1.first; c1 = j1.second;
            r2 = j2.first; c2 = j2.second;
        }

        // Foursquare rule: take from opposite corners
        QChar enc1 = m_trSquare[r1][c2];   // Row from a, Col from b
        QChar enc2 = m_blSquare[r2][c1];   // Row from b, Col from a

        cipher.append(enc1);
        cipher.append(enc2);
        digraphs++;
    }

    result.ciphertext = cipher;
    result.length = cipher.size();
    result.digraphCount = digraphs;
    result.elapsedMs = timer.elapsed();

    m_stats.totalOps++;
    m_stats.totalCharsEncrypted += n;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptDone(result.length, digraphs, result.elapsedMs);
    return result;
}

/* ---- Decrypt ---- */

FoursquareCode16::EncryptResult FoursquareCode16::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    EncryptResult result;
    QString processed = ciphertext.toUpper();
    int n = processed.size();
    if (n < 2) { result.elapsedMs = timer.elapsed(); return result; }

    QString plain;
    int digraphs = 0;

    for (int i = 0; i < n - 1; i += 2) {
        QChar a = processed[i];
        QChar b = processed[i + 1];

        // Find positions in cipher squares
        auto [r1, c1] = findPosition(m_trSquare, a);
        auto [r2, c2] = findPosition(m_blSquare, b);

        // Remove jitter
        if (m_config.jitterStrength > 0.0) {
            auto j1 = removeJitter(r1, c2);
            auto j2 = removeJitter(r2, c1);
            r1 = j1.first; c1 = j2.second;
            r2 = j2.first; c2 = j1.second;
        }

        // Reverse foursquare: use original positions
        QChar dec1 = m_tlSquare[r1][c2];
        QChar dec2 = m_brSquare[r2][c1];

        plain.append(dec1);
        plain.append(dec2);
        digraphs++;
    }

    result.ciphertext = plain;
    result.length = plain.size();
    result.digraphCount = digraphs;
    result.elapsedMs = timer.elapsed();

    m_stats.totalOps++;
    m_stats.totalCharsEncrypted += n;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Frequency analysis ---- */

QMap<QChar, int> FoursquareCode16::frequencyAnalysis(const QString& text) const
{
    QMap<QChar, int> freq;
    for (QChar ch : text.toUpper()) {
        if (ch == QLatin1Char('J')) ch = QLatin1Char('I');
        if (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z'))
            freq[ch]++;
    }
    return freq;
}

/* ---- Reset ---- */

void FoursquareCode16::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
