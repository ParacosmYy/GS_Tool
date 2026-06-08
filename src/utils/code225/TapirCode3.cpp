/**
 * @file TapirCode3.cpp
 * @brief TapirCode3 实现
 *
 * 实现Polybius混合编码与模拟退火N元组频率密码分析。
 */

#include "utils/code225/TapirCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/* ---- Construction / Destruction ---- */

TapirCode3::TapirCode3(QObject *parent) : QObject(parent) {}
TapirCode3::~TapirCode3() = default;

/* ---- Load N-gram frequencies ---- */

void TapirCode3::loadNGramFrequencies(const QString& referenceText, int ngramSize)
{
    m_saConfig.ngramSize = ngramSize;
    m_ngramLogFreq.clear();
    m_ngramTotal = 0;

    QMap<QString, int> counts;
    QString cleaned = preprocess(referenceText);
    for (int i = 0; i <= cleaned.size() - ngramSize; ++i) {
        QString ngram = cleaned.mid(i, ngramSize);
        counts[ngram]++;
        m_ngramTotal++;
    }

    if (m_ngramTotal == 0) return;
    double logTotal = qLn(m_ngramTotal);
    for (auto it = counts.begin(); it != counts.end(); ++it)
        m_ngramLogFreq[it.key()] = qLn(it.value()) - logTotal;
}

/* ---- Set SA config ---- */

void TapirCode3::setSAConfig(const SAConfig& config)
{
    m_saConfig = config;
}

/* ---- Preprocess ---- */

QString TapirCode3::preprocess(const QString& text) const
{
    QString result;
    for (QChar ch : text.toUpper())
        if (ch >= 'A' && ch <= 'Z') result.append(ch);
    return result;
}

/* ---- Build Polybius square ---- */

QVector<QVector<QChar>> TapirCode3::buildPolybiusSquare(const QString& key) const
{
    QVector<QVector<QChar>> square(5, QVector<QChar>(5));
    QVector<bool> used(26, false);
    used['J' - 'A'] = true; // I/J merged

    QString k = preprocess(key);
    int row = 0, col = 0;

    auto place = [&](QChar ch) {
        if (used[ch.toLatin1() - 'A']) return;
        square[row][col] = ch;
        used[ch.toLatin1() - 'A'] = true;
        col++;
        if (col >= 5) { col = 0; row++; }
    };

    for (QChar ch : k) place(ch);
    for (char c = 'A'; c <= 'Z'; ++c) place(QChar(c));
    return square;
}

/* ---- Polybius lookup ---- */

QChar TapirCode3::polybiusLookup(const QVector<QVector<QChar>>& square,
                                   int row, int col) const
{
    return square[row][col];
}

bool TapirCode3::polybiusReverse(const QVector<QVector<QChar>>& square,
                                   QChar ch, int& row, int& col) const
{
    if (ch == 'J') ch = 'I';
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 5; ++c)
            if (square[r][c] == ch) { row = r; col = c; return true; }
    return false;
}

/* ---- Encode ---- */

QString TapirCode3::encode(const QString& plaintext, const QString& key) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<QChar>> square = buildPolybiusSquare(key);
    QString cleaned = preprocess(plaintext);
    QString result;

    // Polybius-hybrid: map each char to (row*5+col), then interleave pairs
    QVector<int> codes;
    for (QChar ch : cleaned) {
        int r, c;
        if (polybiusReverse(square, ch, r, c))
            codes.append(r * 5 + c);
    }

    // Hybrid encoding: take pairs of codes, combine as 2-digit base-5
    for (int i = 0; i + 1 < codes.size(); i += 2) {
        int a = codes[i], b = codes[i + 1];
        int ar = a / 5, ac = a % 5;
        int br = b / 5, bc = b % 5;
        // Interleave: row_a, row_b, col_a, col_b -> two new symbols
        int sym1 = ar * 5 + br;
        int sym2 = ac * 5 + bc;
        result.append(QChar('A' + (sym1 % 26)));
        result.append(QChar('A' + (sym2 % 26)));
    }
    if (codes.size() % 2 != 0)
        result.append(QChar('A' + (codes.last() % 26)));

    const_cast<TapirCode3*>(this)->m_stats.encodeCount++;
    const_cast<TapirCode3*>(this)->m_encodeTimeSum += timer.elapsed();
    const_cast<TapirCode3*>(this)->m_stats.avgEncodingTimeMs =
        m_encodeTimeSum / m_stats.encodeCount;
    const_cast<TapirCode3*>(this)->m_stats.totalOps++;
    emit const_cast<TapirCode3*>(this)->operationCompleted("encode", timer.elapsed());
    return result;
}

/* ---- Random key ---- */

QString TapirCode3::randomKey() const
{
    QVector<int> perm(26);
    for (int i = 0; i < 26; ++i) perm[i] = i;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(perm.begin(), perm.end(), gen);
    QString key;
    for (int i = 0; i < 26; ++i) key.append(QChar('A' + perm[i]));
    return key;
}

/* ---- Mutate key ---- */

QString TapirCode3::mutateKey(const QString& key) const
{
    QString mutated = key;
    int a = qrand() % key.size();
    int b = qrand() % key.size();
    std::swap(mutated[a], mutated[b]);
    return mutated;
}

/* ---- Decode with key ---- */

QString TapirCode3::decodeWithKey(const QString& ciphertext,
                                   const QString& key) const
{
    QVector<QVector<QChar>> square = buildPolybiusSquare(key);
    QString result;
    QString cleaned = preprocess(ciphertext);

    // Reverse hybrid encoding
    QVector<int> codes;
    for (int i = 0; i + 1 < cleaned.size(); i += 2) {
        int sym1 = (cleaned[i].toLatin1() - 'A') % 25;
        int sym2 = (cleaned[i + 1].toLatin1() - 'A') % 25;
        int ar = sym1 / 5, br = sym1 % 5;
        int ac = sym2 / 5, bc = sym2 % 5;
        codes.append(ar * 5 + ac);
        codes.append(br * 5 + bc);
    }

    for (int code : codes) {
        int r = code / 5, c = code % 5;
        if (r < 5 && c < 5) result.append(square[r][c]);
    }
    return result;
}

/* ---- N-gram fitness ---- */

double TapirCode3::ngramFitness(const QString& text) const
{
    if (m_ngramLogFreq.isEmpty()) return 0.0;
    double score = 0.0;
    int n = m_saConfig.ngramSize;
    for (int i = 0; i <= text.size() - n; ++i) {
        QString ng = text.mid(i, n);
        score += m_ngramLogFreq.value(ng, -15.0);
    }
    return score;
}

/* ---- Score text ---- */

double TapirCode3::scoreText(const QString& text) const
{
    return ngramFitness(preprocess(text));
}

/* ---- Simulated annealing decode ---- */

TapirCode3::DecodeResult TapirCode3::decode(const QString& ciphertext,
                                             const QString& initialKey) const
{
    QElapsedTimer timer;
    timer.start();

    QString bestKey = initialKey.isEmpty() ? randomKey() : initialKey;
    QString bestPlaintext = decodeWithKey(ciphertext, bestKey);
    double bestFitness = ngramFitness(bestPlaintext);

    QString currentKey = bestKey;
    double currentFitness = bestFitness;
    double temp = m_saConfig.initialTemp;

    for (int iter = 0; iter < m_saConfig.maxIterations; ++iter) {
        QString newKey = mutateKey(currentKey);
        QString newPlain = decodeWithKey(ciphertext, newKey);
        double newFit = ngramFitness(newPlain);

        double delta = newFit - currentFitness;
        if (delta > 0 || qExp(delta / qMax(temp, 1e-10)) > (qrand() % 10000) / 10000.0) {
            currentKey = newKey;
            currentFitness = newFit;
        }
        if (currentFitness > bestFitness) {
            bestFitness = currentFitness;
            bestKey = currentKey;
            bestPlaintext = decodeWithKey(ciphertext, bestKey);
        }
        temp *= m_saConfig.coolingRate;
        if (iter % 5000 == 0)
            emit const_cast<TapirCode3*>(this)->decodeProgress(iter, bestFitness, temp);
    }

    DecodeResult result;
    result.plaintext = bestPlaintext;
    result.bestKey = bestKey;
    result.fitness = bestFitness;
    result.iterations = m_saConfig.maxIterations;

    const_cast<TapirCode3*>(this)->m_stats.decodeCount++;
    const_cast<TapirCode3*>(this)->m_decodeTimeSum += timer.elapsed();
    const_cast<TapirCode3*>(this)->m_stats.avgDecodingTimeMs =
        m_decodeTimeSum / m_stats.decodeCount;
    const_cast<TapirCode3*>(this)->m_stats.bestFitness =
        qMax(m_stats.bestFitness, bestFitness);
    const_cast<TapirCode3*>(this)->m_stats.totalOps++;
    return result;
}

/* ---- Reset ---- */

void TapirCode3::resetStatistics()
{
    m_stats = Stats{};
    m_encodeTimeSum = 0.0;
    m_decodeTimeSum = 0.0;
    m_ngramLogFreq.clear();
    m_ngramTotal = 0;
}
