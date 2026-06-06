/**
 * @file RailFenceCode.cpp
 * @brief RailFenceCode 实现
 *
 * 实现栅栏密码：锯齿读写模式、可配置轨道数、暴力穷举破解、频率评分。
 */

#include "utils/code179/RailFenceCode.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- English letter frequencies (A-Z) ---- */

const QVector<double> RailFenceCode::s_englishFreq = {
    0.0817, 0.0150, 0.0278, 0.0425, 0.1270, 0.0223, 0.0202,
    0.0609, 0.0697, 0.0015, 0.0077, 0.0403, 0.0241, 0.0675,
    0.0751, 0.0193, 0.0010, 0.0599, 0.0633, 0.0906, 0.0276,
    0.0098, 0.0236, 0.0015, 0.0197, 0.0007
};

/* ---- Construction / Destruction ---- */

RailFenceCode::RailFenceCode(QObject *parent) : QObject(parent) {}
RailFenceCode::~RailFenceCode() = default;

/* ---- Configuration ---- */

void RailFenceCode::setRails(int rails) { m_rails = qMax(2, rails); }

/* ---- Build zigzag matrix ---- */

QVector<QVector<QChar>> RailFenceCode::buildZigzagMatrix(const QString& text) const
{
    int n = text.size();
    int r = m_rails;
    QVector<QVector<QChar>> matrix(r, QVector<QChar>(n, QChar()));

    int row = 0;
    int dir = 1; // 1 = down, -1 = up
    for (int col = 0; col < n; ++col) {
        matrix[row][col] = text[col];
        if (row == 0) dir = 1;
        else if (row == r - 1) dir = -1;
        row += dir;
    }
    return matrix;
}

/* ---- Read off rails row-by-row ---- */

QString RailFenceCode::readOffRails(const QVector<QVector<QChar>>& matrix) const
{
    QString result;
    for (int r = 0; r < matrix.size(); ++r)
        for (int c = 0; c < matrix[r].size(); ++c)
            if (!matrix[r][c].isNull())
                result.append(matrix[r][c]);
    return result;
}

/* ---- Encrypt ---- */

QString RailFenceCode::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    if (plaintext.isEmpty()) return {};

    auto matrix = buildZigzagMatrix(plaintext);
    QString result = readOffRails(matrix);

    m_stats.totalEncryptions++;
    m_stats.lastRails = m_rails;
    m_stats.lastLength = plaintext.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit encryptionCompleted(m_rails, plaintext.size());
    return result;
}

/* ---- Decode from rail-read ciphertext ---- */

QString RailFenceCode::decodeFromRails(const QString& ciphertext, int rails) const
{
    int n = ciphertext.size();
    if (n == 0) return {};

    // Compute length of each rail
    QVector<int> railLen(rails, 0);
    int row = 0, dir = 1;
    for (int i = 0; i < n; ++i) {
        railLen[row]++;
        if (row == 0) dir = 1;
        else if (row == rails - 1) dir = -1;
        row += dir;
    }

    // Fill rails from ciphertext
    QVector<QVector<QChar>> railChars(rails);
    int idx = 0;
    for (int r = 0; r < rails; ++r) {
        for (int j = 0; j < railLen[r]; ++j)
            railChars[r].append(ciphertext[idx++]);
    }

    // Read back in zigzag order
    QString result;
    QVector<int> railIdx(rails, 0);
    row = 0; dir = 1;
    for (int i = 0; i < n; ++i) {
        result.append(railChars[row][railIdx[row]]);
        railIdx[row]++;
        if (row == 0) dir = 1;
        else if (row == rails - 1) dir = -1;
        row += dir;
    }
    return result;
}

/* ---- Decrypt ---- */

QString RailFenceCode::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    if (ciphertext.isEmpty()) return {};

    QString result = decodeFromRails(ciphertext, m_rails);

    m_stats.totalDecryptions++;
    m_stats.lastRails = m_rails;
    m_stats.lastLength = ciphertext.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit decryptionCompleted(m_rails, ciphertext.size());
    return result;
}

/* ---- Frequency score ---- */

double RailFenceCode::frequencyScore(const QString& text) const
{
    if (text.isEmpty()) return 0.0;

    int totalLetters = 0;
    QVector<int> counts(26, 0);
    for (const QChar& c : text) {
        if (c.isLetter()) {
            int idx = c.toUpper().unicode() - 'A';
            if (idx >= 0 && idx < 26) { counts[idx]++; totalLetters++; }
        }
    }
    if (totalLetters == 0) return 0.0;

    // Chi-squared against English frequency
    double chi2 = 0.0;
    for (int i = 0; i < 26; ++i) {
        double expected = s_englishFreq[i] * totalLetters;
        if (expected > 0.0) {
            double diff = counts[i] - expected;
            chi2 += (diff * diff) / expected;
        }
    }
    // Lower chi-squared = better fit; return inverse as score
    return 1.0 / (1.0 + chi2);
}

/* ---- Brute-force crack ---- */

QVector<RailFenceCode::CrackResult> RailFenceCode::bruteForceCrack(
    const QString& ciphertext, int maxRails)
{
    int maxR = maxRails > 0 ? maxRails : qMax(2, ciphertext.size() / 2);
    QVector<CrackResult> results;

    for (int r = 2; r <= maxR; ++r) {
        CrackResult cr;
        cr.rails = r;
        cr.plaintext = decodeFromRails(ciphertext, r);
        cr.score = frequencyScore(cr.plaintext);
        results.append(cr);
    }

    // Sort by descending score
    std::sort(results.begin(), results.end(),
              [](const CrackResult& a, const CrackResult& b) {
                  return a.score > b.score;
              });

    return results;
}

/* ---- Reset ---- */

void RailFenceCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
