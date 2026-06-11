/**
 * @file SeriatedPlayfair8.cpp
 * @brief SeriatedPlayfair8 实现
 *
 * 实现序列化Playfair密码：渐进关键字旋转与双字母频率平衡增强波利比乌斯加密。
 */

#include "utils/code290/SeriatedPlayfair8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

SeriatedPlayfair8::SeriatedPlayfair8(QObject *parent)
    : QObject(parent)
{
    buildGrid(QStringLiteral("KEYWORD"));
}

SeriatedPlayfair8::~SeriatedPlayfair8() = default;

/* ---- Configuration ---- */

void SeriatedPlayfair8::setKeyword(const QString& keyword) { m_keyword = keyword; buildGrid(keyword); }
void SeriatedPlayfair8::setRotationPeriod(int period) { m_rotationPeriod = qBound(1, period, 1000); }
void SeriatedPlayfair8::setFillOrder(int order) { m_fillOrder = qBound(0, order, 2); buildGrid(m_keyword); }

/* ---- Normalize: J→I, uppercase ---- */

QChar SeriatedPlayfair8::normalize(QChar ch) const
{
    ch = ch.toUpper();
    if (ch == QLatin1Char('J')) return QLatin1Char('I');
    if (ch >= QLatin1Char('A') && ch <= QLatin1Char('Z')) return ch;
    return QChar::Null;
}

/* ---- Build 5×5 grid from keyword ---- */

void SeriatedPlayfair8::buildGrid(const QString& keyword)
{
    m_grid = QVector<QVector<QChar>>(5, QVector<QChar>(5));
    m_positionMap.resize(26);
    m_positionMap.fill(-1);

    QVector<QChar> chars;
    // Add keyword letters first (unique)
    for (QChar ch : keyword.toUpper()) {
        if (ch == QLatin1Char('J')) ch = QLatin1Char('I');
        if (ch < QLatin1Char('A') || ch > QLatin1Char('Z')) continue;
        int idx = ch.toLatin1() - 'A';
        if (m_positionMap[idx] >= 0) continue;
        m_positionMap[idx] = chars.size();
        chars.append(ch);
    }
    // Fill remaining alphabet
    for (int c = 0; c < 26; ++c) {
        if (c == 9) continue; // Skip J
        if (m_positionMap[c] >= 0) continue;
        m_positionMap[c] = chars.size();
        chars.append(QChar(QLatin1Char('A' + c)));
    }

    // Fill grid based on fill order
    int ci = 0;
    if (m_fillOrder == 0) {
        // Row-major
        for (int r = 0; r < 5; ++r)
            for (int c = 0; c < 5; ++c)
                m_grid[r][c] = chars[ci++];
    } else if (m_fillOrder == 1) {
        // Column-major
        for (int c = 0; c < 5; ++c)
            for (int r = 0; r < 5; ++r)
                m_grid[r][c] = chars[ci++];
    } else {
        // Spiral fill
        int r = 0, c = 0, dr = 0, dc = 1;
        for (int i = 0; i < 25; ++i) {
            m_grid[r][c] = chars[ci++];
            int nr = r + dr, nc = c + dc;
            if (nr < 0 || nr >= 5 || nc < 0 || nc >= 5 || m_grid[nr][nc] != QChar::Null) {
                int tmp = dr; dr = dc; dc = -tmp;
                nr = r + dr; nc = c + dc;
            }
            r = nr; c = nc;
        }
    }

    // Rebuild position map from actual grid
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c) {
            int idx = m_grid[r][c].toLatin1() - 'A';
            m_positionMap[idx] = r * 5 + c;
        }
}

/* ---- Rotate grid by shifting rows ---- */

void SeriatedPlayfair8::rotateGrid(int step)
{
    // Left-rotate each row by step positions (progressive keyword rotation)
    for (int r = 0; r < 5; ++r) {
        QVector<QChar> row = m_grid[r];
        int s = (step + r) % 5; // Different shift per row for seriation
        for (int c = 0; c < 5; ++c)
            m_grid[r][c] = row[(c + s) % 5];
    }
    // Rebuild position map
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c) {
            int idx = m_grid[r][c].toLatin1() - 'A';
            m_positionMap[idx] = r * 5 + c;
        }
}

/* ---- Find position in grid ---- */

QPair<int,int> SeriatedPlayfair8::findPosition(QChar ch) const
{
    int idx = ch.toLatin1() - 'A';
    int pos = m_positionMap[idx];
    return {pos / 5, pos % 5};
}

/* ---- Encode digraph ---- */

QPair<QChar,QChar> SeriatedPlayfair8::encodeDigraph(QChar a, QChar b) const
{
    auto [r1, c1] = findPosition(a);
    auto [r2, c2] = findPosition(b);

    if (r1 == r2) {
        // Same row: shift right
        return {m_grid[r1][(c1 + 1) % 5], m_grid[r2][(c2 + 1) % 5]};
    } else if (c1 == c2) {
        // Same column: shift down
        return {m_grid[(r1 + 1) % 5][c1], m_grid[(r2 + 1) % 5][c2]};
    } else {
        // Rectangle: swap columns
        return {m_grid[r1][c2], m_grid[r2][c1]};
    }
}

/* ---- Decode digraph ---- */

QPair<QChar,QChar> SeriatedPlayfair8::decodeDigraph(QChar a, QChar b) const
{
    auto [r1, c1] = findPosition(a);
    auto [r2, c2] = findPosition(b);

    if (r1 == r2) {
        return {m_grid[r1][(c1 + 4) % 5], m_grid[r2][(c2 + 4) % 5]};
    } else if (c1 == c2) {
        return {m_grid[(r1 + 4) % 5][c1], m_grid[(r2 + 4) % 5][c2]};
    } else {
        return {m_grid[r1][c2], m_grid[r2][c1]};
    }
}

/* ---- Prepare text ---- */

QString SeriatedPlayfair8::prepareText(const QString& text) const
{
    QString result;
    for (QChar ch : text) {
        QChar n = normalize(ch);
        if (!n.isNull()) result.append(n);
    }
    // Insert X between repeated letters
    QString padded;
    for (int i = 0; i < result.size(); ++i) {
        padded.append(result[i]);
        if (i + 1 < result.size() && result[i] == result[i + 1])
            padded.append(QLatin1Char('X'));
    }
    if (padded.size() % 2 != 0)
        padded.append(QLatin1Char('X'));
    return padded;
}

/* ---- Encrypt ---- */

SeriatedPlayfair8::EncResult SeriatedPlayfair8::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    EncResult result;
    buildGrid(m_keyword);

    QString prepared = prepareText(plaintext);
    QString cipher;
    int rotations = 0;
    int digraphCount = 0;

    for (int i = 0; i < prepared.size(); i += 2) {
        QChar a = prepared[i];
        QChar b = (i + 1 < prepared.size()) ? prepared[i + 1] : QLatin1Char('X');

        auto [ca, cb] = encodeDigraph(a, b);
        cipher.append(ca);
        cipher.append(cb);
        digraphCount++;

        // Progressive keyword rotation
        if (digraphCount % m_rotationPeriod == 0) {
            rotateGrid(digraphCount / m_rotationPeriod);
            rotations++;
        }
    }

    result.cipherText = cipher;
    result.gridRotations = rotations;
    result.digraphBalance = 0.0;

    // Compute digraph balance metric
    auto freqs = analyzeDigraphFrequency(cipher);
    if (!freqs.isEmpty()) {
        double mean = 1.0 / freqs.size();
        double variance = 0.0;
        for (const auto& p : freqs)
            variance += (p.second - mean) * (p.second - mean);
        result.digraphBalance = variance / freqs.size();
    }

    double elapsed = timer.elapsed();
    m_stats.charsEncrypted += prepared.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit encryptDone(cipher.size(), result.digraphBalance, elapsed);
    return result;
}

/* ---- Decrypt ---- */

SeriatedPlayfair8::EncResult SeriatedPlayfair8::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    EncResult result;
    buildGrid(m_keyword);

    QString plain;
    int rotations = 0;
    int digraphCount = 0;

    for (int i = 0; i < ciphertext.size(); i += 2) {
        QChar a = normalize(ciphertext[i]);
        QChar b = (i + 1 < ciphertext.size()) ? normalize(ciphertext[i + 1]) : QLatin1Char('X');

        auto [pa, pb] = decodeDigraph(a, b);
        plain.append(pa);
        plain.append(pb);
        digraphCount++;

        if (digraphCount % m_rotationPeriod == 0) {
            rotateGrid(digraphCount / m_rotationPeriod);
            rotations++;
        }
    }

    result.cipherText = plain;
    result.gridRotations = rotations;

    double elapsed = timer.elapsed();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    return result;
}

/* ---- Digraph frequency analysis ---- */

QVector<QPair<QString,double>> SeriatedPlayfair8::analyzeDigraphFrequency(const QString& text) const
{
    QMap<QString, int> counts;
    int total = 0;
    for (int i = 0; i + 1 < text.size(); i += 2) {
        QString dg = text.mid(i, 2);
        counts[dg]++;
        total++;
    }
    QVector<QPair<QString,double>> result;
    for (auto it = counts.begin(); it != counts.end(); ++it)
        result.append({it.key(), static_cast<double>(it.value()) / qMax(total, 1)});
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    return result;
}

/* ---- Get current grid ---- */

QVector<QVector<QChar>> SeriatedPlayfair8::getGrid() const { return m_grid; }

/* ---- Reset ---- */

void SeriatedPlayfair8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    buildGrid(m_keyword);
}
