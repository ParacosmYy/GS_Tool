/**
 * @file BCHCode4.cpp
 * @brief BCHCode4 实现
 *
 * 实现BCH循环码：GF(2^m)域运算、生成多项式构造、
 * 系统编码、Berlekamp-Massey解码和Chien搜索纠错。
 */

#include "utils/code162/BCHCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数：初始化GF域和生成多项式
 */
BCHCode4::BCHCode4(int m, int t, QObject* parent)
    : QObject(parent)
    , m_m(qMax(2, m))
    , m_t(qMax(1, t))
    , m_n((1 << m_m) - 1)
{
    buildGField();
    m_k = m_n - m_generator.size() + 1;
}

BCHCode4::~BCHCode4() = default;

/**
 * @brief 初始化GF(2^m)域的指数表和对数表
 *
 * 使用本原多项式构建。指数表: gfExp[i] = alpha^i。
 * 对数表: gfLog[x] = log_alpha(x)。
 */
void BCHCode4::buildGField()
{
    const int fieldSize = (1 << m_m);
    m_gfExp.resize(2 * m_n);
    m_gfLog.resize(fieldSize);

    /* Primitive polynomial coefficients (minimal polynomials for small m) */
    /* Common primitive polynomials: m=3: x^3+x+1=0xB, m=4: x^4+x+1=0x13, etc. */
    int primPoly = 0;
    switch (m_m) {
    case 3:  primPoly = 0x0B; break;   /* x^3 + x + 1 */
    case 4:  primPoly = 0x13; break;   /* x^4 + x + 1 */
    case 5:  primPoly = 0x25; break;   /* x^5 + x^2 + 1 */
    case 6:  primPoly = 0x43; break;   /* x^6 + x + 1 */
    case 7:  primPoly = 0x89; break;   /* x^7 + x^3 + 1 */
    case 8:  primPoly = 0x11D; break;  /* x^8 + x^4 + x^3 + x^2 + 1 */
    default: primPoly = 0x13; break;
    }

    int val = 1;
    m_gfLog[0] = -1;
    for (int i = 0; i < m_n; ++i) {
        m_gfExp[i] = val;
        m_gfLog[val] = i;
        val <<= 1;
        if (val >= fieldSize) val ^= primPoly;
    }
    /* Double the exp table for easy modular access */
    for (int i = 0; i < m_n; ++i) {
        m_gfExp[m_n + i] = m_gfExp[i];
    }

    buildGeneratorPolynomial();
}

/**
 * @brief GF(2^m)域元素乘法
 */
int BCHCode4::gfMul(int a, int b) const
{
    if (a == 0 || b == 0) return 0;
    return m_gfExp[m_gfLog[a] + m_gfLog[b]];
}

/**
 * @brief GF(2^m)域元素求逆
 */
int BCHCode4::gfInverse(int a) const
{
    if (a == 0) return 0;
    return m_gfExp[m_n - m_gfLog[a]];
}

/**
 * @brief 构造生成多项式
 *
 * LCM of minimal polynomials of alpha, alpha^3, ..., alpha^(2t-1).
 * Use GF(2) polynomial multiplication (XOR-based).
 */
void BCHCode4::buildGeneratorPolynomial()
{
    /* Start with g(x) = 1 */
    m_generator = {1};

    for (int i = 1; i <= 2 * m_t; i += 2) {
        /* Minimal polynomial of alpha^i: find the conjugate roots */
        QVector<int> minPoly = {1};
        int root = m_gfExp[i % m_n];

        /* Build (x - alpha^i) in GF(2) => x + alpha^i since GF(2) addition = XOR */
        /* For BCH over GF(2), minimal poly has coefficients in {0,1} */
        /* Find all conjugates: alpha^i, alpha^(2i), alpha^(4i), ... */
        QVector<int> roots;
        int r = i % m_n;
        do {
            roots.append(r);
            r = (2 * r) % m_n;
        } while (r != (i % m_n));

        /* Compute minimal polynomial as product of (x - alpha^root) for each root */
        minPoly = {1};
        for (int ri : roots) {
            int alphaPow = m_gfExp[ri];
            QVector<int> newPoly(minPoly.size() + 1, 0);
            for (int j = 0; j < minPoly.size(); ++j) {
                /* Multiply by (x - alpha^root) over GF(2) */
                newPoly[j] ^= minPoly[j];
                newPoly[j + 1] ^= minPoly[j];
            }
            minPoly = newPoly;
        }

        /* Multiply g(x) *= minPoly over GF(2) */
        QVector<int> product(m_generator.size() + minPoly.size() - 1, 0);
        for (int a = 0; a < m_generator.size(); ++a) {
            for (int b = 0; b < minPoly.size(); ++b) {
                product[a + b] ^= m_generator[a] & minPoly[b];
            }
        }
        m_generator = product;
    }
}

/**
 * @brief 编码：系统编码
 *
 * 将信息位放在高位，低位追加校验位。
 */
QVector<int> BCHCode4::encode(const QVector<int>& message) const
{
    if (message.size() != m_k) return QVector<int>();

    /* Multiply message polynomial by x^(n-k) */
    QVector<int> codeword(m_n, 0);
    for (int i = 0; i < m_k; ++i) {
        codeword[i] = message[i];
    }

    /* Compute remainder: shift message bits XOR with generator */
    QVector<int> reg(m_generator.size(), 0);
    for (int i = 0; i < m_k; ++i) {
        int feedback = codeword[i] ^ reg[0];
        reg.removeFirst();
        reg.append(0);
        if (feedback) {
            for (int j = 0; j < m_generator.size(); ++j) {
                reg[j] ^= m_generator[j];
            }
        }
    }

    /* Append parity bits */
    for (int i = 0; i < m_generator.size() - 1; ++i) {
        codeword[m_k + i] = reg[i];
    }

    return codeword;
}

/**
 * @brief 计算伴随式
 *
 * S_j = r(alpha^j), j = 1, 2, ..., 2t
 */
QVector<int> BCHCode4::computeSyndromes(const QVector<int>& received) const
{
    QVector<int> syndromes(2 * m_t + 1);
    for (int j = 1; j <= 2 * m_t; ++j) {
        int val = 0;
        int alphaJ = m_gfExp[j];
        int alphaPow = 1;
        for (int i = 0; i < received.size(); ++i) {
            if (received[i]) val ^= alphaPow;
            alphaPow = gfMul(alphaPow, alphaJ);
        }
        syndromes[j] = val;
    }
    return syndromes;
}

/**
 * @brief Berlekamp-Massey算法
 *
 * 求解最短LFSR(错误定位多项式)。
 */
QVector<int> BCHCode4::berlekampMassey(const QVector<int>& syndromes) const
{
    const int numSyn = 2 * m_t;
    QVector<int> sigma(numSyn + 1, 0);
    sigma[0] = 1;
    QVector<int> oldSigma = sigma;
    int L = 0;

    for (int n = 1; n <= numSyn; ++n) {
        /* Compute discrepancy */
        int disc = syndromes[n];
        for (int i = 1; i <= L; ++i) {
            disc = gfAdd(disc, gfMul(sigma[i], syndromes[n - i]));
        }

        if (disc == 0) continue;

        QVector<int> newSigma = sigma;
        /* sigma = sigma - disc * x * oldSigma */
        QVector<int> temp(numSyn + 1, 0);
        for (int i = 0; i < numSyn; ++i) {
            temp[i + 1] = gfMul(disc, oldSigma[i]);
        }
        for (int i = 0; i <= numSyn; ++i) {
            sigma[i] = gfAdd(sigma[i], temp[i]);
        }

        if (2 * L <= n - 1) {
            L = n - L;
            oldSigma = newSigma;
        } else {
            oldSigma = newSigma;
        }
    }

    sigma.resize(L + 1);
    return sigma;
}

/**
 * @brief Chien搜索：代入alpha^(-i)找错误位置
 */
QVector<int> BCHCode4::chienSearch(const QVector<int>& errorLocator) const
{
    const int deg = errorLocator.size() - 1;
    QVector<int> errorPositions;

    for (int i = 0; i < m_n; ++i) {
        /* Evaluate error locator at alpha^(-i) = alpha^(n-i) */
        int val = 0;
        int alphaPow = 1;
        int alphaI = m_gfExp[(m_n - i) % m_n];

        for (int j = 0; j <= deg; ++j) {
            val = gfAdd(val, gfMul(errorLocator[j], alphaPow));
            alphaPow = gfMul(alphaPow, alphaI);
        }

        if (val == 0) {
            errorPositions.append(i);
        }
    }

    return errorPositions;
}

/**
 * @brief 解码：伴随式计算 -> BM -> Chien搜索 -> 纠错
 */
QVector<int> BCHCode4::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    if (received.size() != m_n) return QVector<int>();

    QVector<int> syndromes = computeSyndromes(received);

    /* Check if all syndromes are zero (no errors) */
    bool hasError = false;
    for (int j = 1; j <= 2 * m_t; ++j) {
        if (syndromes[j] != 0) { hasError = true; break; }
    }

    QVector<int> corrected = received;

    if (hasError) {
        QVector<int> sigma = berlekampMassey(syndromes);
        QVector<int> errorPos = chienSearch(sigma);

        if (errorPos.size() > m_t || errorPos.isEmpty()) {
            m_stats.totalDecodes++;
            m_stats.decodeFailures++;
            m_timeSum += timer.elapsed();
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodes;
            emit decodeCompleted(0, false);
            return QVector<int>();
        }

        /* Flip error bits */
        for (int pos : errorPos) {
            if (pos >= 0 && pos < m_n) {
                corrected[pos] ^= 1;
            }
        }

        m_stats.totalErrorsCorrected += errorPos.size();
        m_stats.totalDecodes++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodes;
        emit decodeCompleted(errorPos.size(), true);
    } else {
        m_stats.totalDecodes++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodes;
        emit decodeCompleted(0, true);
    }

    /* Extract message bits */
    QVector<int> message(m_k);
    for (int i = 0; i < m_k; ++i) {
        message[i] = corrected[i];
    }

    return message;
}

void BCHCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
