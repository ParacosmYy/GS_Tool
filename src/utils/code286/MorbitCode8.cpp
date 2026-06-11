/**
 * @file MorbitCode8.cpp
 * @brief MorbitCode8 实现
 *
 * 实现Morbit密码：扩展符号表与多层替换手操野战加密。
 */

#include "utils/code286/MorbitCode8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

MorbitCode8::MorbitCode8(QObject *parent)
    : QObject(parent)
{
    setConfig(MorbitConfig{});
}

MorbitCode8::~MorbitCode8() = default;

/* ---- Configuration ---- */

void MorbitCode8::setConfig(const MorbitConfig& cfg)
{
    m_config = cfg;
    buildPolybiusSquare();
    buildLayerTables();
}

/* ---- Build 5x5 Polybius square from key ---- */

void MorbitCode8::buildPolybiusSquare()
{
    m_polybiusMap.clear();
    m_polybiusReverse.clear();

    // Build alphabet: key (dedup) + remaining letters (skip J)
    QString alpha;
    QString used;
    QString keyUpper = m_config.key.toUpper();
    for (QChar ch : keyUpper) {
        if (ch == QLatin1Char('J')) ch = QLatin1Char('I');
        if (ch.isLetter() && !used.contains(ch)) { used.append(ch); alpha.append(ch); }
    }
    for (char c = 'A'; c <= 'Z'; ++c) {
        QChar ch(c);
        if (ch == QLatin1Char('J')) continue;
        if (!used.contains(ch)) alpha.append(ch);
    }

    // Fill 5x5 square
    int idx = 0;
    for (int row = 0; row < 5 && idx < alpha.size(); ++row) {
        for (int col = 0; col < 5 && idx < alpha.size(); ++col) {
            QChar ch = alpha[idx++];
            m_polybiusMap[ch] = row * 10 + col;  // Encode as two-digit
            m_polybiusReverse[row * 10 + col] = ch;
        }
    }
    // Add digit mappings for extended alphabet
    for (char c = '0'; c <= '9'; ++c) {
        if (idx < 25) continue; // Only extend beyond 25 if needed
        QChar ch(c);
        m_polybiusMap[ch] = 50 + (c - '0');
        m_polybiusReverse[50 + (c - '0')] = ch;
    }
}

/* ---- Build multi-layer substitution tables ---- */

void MorbitCode8::buildLayerTables()
{
    int base = m_config.symbolBase;
    int layers = m_config.numLayers;
    m_layerSubs.resize(layers);
    m_layerSubsInv.resize(layers);

    std::mt19937 rng(m_config.key.isEmpty() ? 42 : m_config.key[0].unicode());
    for (int l = 0; l < layers; ++l) {
        m_layerSubs[l].resize(base);
        m_layerSubsInv[l].resize(base);
        // Generate permutation seeded by key and layer
        for (int i = 0; i < base; ++i) m_layerSubs[l][i] = i;
        // Fisher-Yates shuffle
        for (int i = base - 1; i > 0; --i) {
            std::uniform_int_distribution<int> dist(0, i);
            int j = dist(rng);
            std::swap(m_layerSubs[l][i], m_layerSubs[l][j]);
        }
        for (int i = 0; i < base; ++i) m_layerSubsInv[l][m_layerSubs[l][i]] = i;
    }
}

/* ---- Polybius encode ---- */

QVector<int> MorbitCode8::polybiusEncode(QChar ch) const
{
    QVector<int> result;
    ch = ch.toUpper();
    if (ch == QLatin1Char('J')) ch = QLatin1Char('I');
    if (m_polybiusMap.contains(ch)) {
        int code = m_polybiusMap[ch];
        result.append(code / 10);
        result.append(code % 10);
    }
    return result;
}

/* ---- Polybius decode ---- */

QChar MorbitCode8::polybiusDecode(int row, int col) const
{
    int code = row * 10 + col;
    return m_polybiusReverse.value(code, QLatin1Char('?'));
}

/* ---- Apply forward substitution layer ---- */

QVector<int> MorbitCode8::applyLayer(const QVector<int>& digits, int layer) const
{
    QVector<int> result;
    result.reserve(digits.size());
    int base = m_config.symbolBase;
    for (int d : digits) {
        if (d >= 0 && d < base && layer < m_layerSubs.size())
            result.append(m_layerSubs[layer][d]);
        else
            result.append(d % base);
    }
    return result;
}

/* ---- Apply inverse substitution layer ---- */

QVector<int> MorbitCode8::applyLayerInverse(const QVector<int>& digits, int layer) const
{
    QVector<int> result;
    result.reserve(digits.size());
    int base = m_config.symbolBase;
    for (int d : digits) {
        if (d >= 0 && d < base && layer < m_layerSubsInv.size())
            result.append(m_layerSubsInv[layer][d % base]);
        else
            result.append(d);
    }
    return result;
}

/* ---- Digits to symbols ---- */

QString MorbitCode8::digitsToSymbols(const QVector<int>& digits) const
{
    QString result;
    if (m_config.symbolBase <= 10) {
        for (int d : digits) result.append(QChar('0' + (d % m_config.symbolBase)));
    } else {
        for (int d : digits) {
            if (d < 10) result.append(QChar('0' + d));
            else result.append(QChar('A' + d - 10));
        }
    }
    return result;
}

/* ---- Symbols to digits ---- */

QVector<int> MorbitCode8::symbolsToDigits(const QString& sym) const
{
    QVector<int> result;
    for (QChar ch : sym) {
        if (ch.isDigit()) result.append(ch.digitValue());
        else if (ch.isUpper()) result.append(10 + (ch.toLatin1() - 'A'));
        else if (ch.isLower()) result.append(10 + (ch.toUpper().toLatin1() - 'A'));
        else result.append(0);
    }
    return result;
}

/* ---- Encrypt ---- */

MorbitCode8::EncryptResult MorbitCode8::encrypt(const QString& plainText)
{
    QElapsedTimer timer;
    timer.start();

    EncryptResult result;
    result.symbolBase = m_config.symbolBase;
    result.numLayers = m_config.numLayers;

    // Step 1: Polybius encode each character to digit pairs
    QVector<int> digits;
    for (QChar ch : plainText) {
        QVector<int> encoded = polybiusEncode(ch);
        if (encoded.size() == 2) {
            digits.append(encoded);
        } else {
            // Non-alpha: encode as space marker
            digits.append(4);
            digits.append(ch.unicode() % m_config.symbolBase);
        }
    }

    // Step 2: Apply multi-layer substitution
    for (int l = 0; l < m_config.numLayers; ++l)
        digits = applyLayer(digits, l);

    result.intermediateDigit = digitsToSymbols(digits);
    result.cipherText = result.intermediateDigit;

    double elapsed = timer.elapsed();
    m_stats.totalCharsEncrypted += plainText.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit encryptDone(plainText.size(), result.cipherText.size(), elapsed);
    return result;
}

/* ---- Decrypt ---- */

QString MorbitCode8::decrypt(const QString& cipherText)
{
    QElapsedTimer timer;
    timer.start();

    // Step 1: Convert symbols to digits
    QVector<int> digits = symbolsToDigits(cipherText);

    // Step 2: Reverse multi-layer substitution (reverse order)
    for (int l = m_config.numLayers - 1; l >= 0; --l)
        digits = applyLayerInverse(digits, l);

    // Step 3: Polybius decode digit pairs
    QString plainText;
    for (int i = 0; i + 1 < digits.size(); i += 2) {
        int row = digits[i];
        int col = digits[i + 1];
        if (row < 5 && col < 5) {
            plainText.append(polybiusDecode(row, col));
        } else {
            plainText.append(QLatin1Char(' '));
        }
    }

    double elapsed = timer.elapsed();
    m_stats.totalCharsDecrypted += plainText.size();
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decryptDone(cipherText.size(), plainText.size(), elapsed);
    return plainText;
}

/* ---- Generate key ---- */

QString MorbitCode8::generateKey(int length) const
{
    std::mt19937 rng(static_cast<unsigned>(QDateTime::currentMSecsSinceEpoch()));
    QString alpha = QStringLiteral("ABCDEFGHIKLMNOPQRSTUVWXYZ");
    QString key;
    std::uniform_int_distribution<int> dist(0, alpha.size() - 1);
    for (int i = 0; i < length; ++i) key.append(alpha[dist(rng)]);
    return key;
}

/* ---- Reset ---- */

void MorbitCode8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
