/**
 * @file BchCode.cpp
 * @brief BCH编解码器实现 — Berlekamp-Massey纠错
 */

#include "utils/code10/BchCode.h"

#include <QElapsedTimer>

#include <cmath>
#include <algorithm>

/** @brief 构造函数 @param m GF(2^m)的m值 @param t 纠错能力 @param parent 父对象 */
BchCode::BchCode(int m, int t, QObject* parent)
    : QObject(parent)
    , m_m(m)
    , m_t(t)
    , m_n((1 << m) - 1)
{
    initGaloisField();
    computeGeneratorPolynomial();
}

/** @brief 初始化GF(2^m)对数/反对数表 */
void BchCode::initGaloisField()
{
    int fieldSize = (1 << m_m);
    m_alphaTo.resize(fieldSize);
    m_indexOf.resize(fieldSize);

    /* 本原多项式(按m值选取常见本原多项式) */
    int primitivePoly = 0;
    switch (m_m) {
    case 3: primitivePoly = 0x0B; break;  /* x^3 + x + 1 */
    case 4: primitivePoly = 0x13; break;  /* x^4 + x + 1 */
    case 5: primitivePoly = 0x25; break;  /* x^5 + x^2 + 1 */
    case 6: primitivePoly = 0x43; break;  /* x^6 + x + 1 */
    case 7: primitivePoly = 0x83; break;  /* x^7 + x + 1 */
    case 8: primitivePoly = 0x11D; break; /* x^8 + x^4 + x^3 + x^2 + 1 */
    default: primitivePoly = 0x13; break;
    }

    /* 生成alpha幂次表 */
    m_alphaTo[0] = 1;
    for (int i = 1; i < fieldSize - 1; ++i) {
        m_alphaTo[i] = m_alphaTo[i - 1] << 1;
        if (m_alphaTo[i] >= (1 << m_m)) {
            m_alphaTo[i] ^= primitivePoly;
            m_alphaTo[i] &= (fieldSize - 1);
        }
    }
    m_alphaTo[fieldSize - 1] = 0;

    /* 构建对数表(indexOf[alpha^i] = i) */
    m_indexOf[0] = -1;
    for (int i = 0; i < fieldSize - 1; ++i) {
        m_indexOf[m_alphaTo[i]] = i;
    }
}

/** @brief 计算生成多项式 */
void BchCode::computeGeneratorPolynomial()
{
    /* g(x) = LCM{最小多项式 of alpha, alpha^3, ..., alpha^(2t-1)} */
    int numRoots = 2 * m_t;

    /* 收集根: alpha^1, alpha^3, ..., alpha^(2t-1) */
    QVector<bool> rootSet(m_n + 1, false);
    for (int i = 1; i <= numRoots; i += 2) {
        int root = m_alphaTo[i % m_n];
        rootSet[root] = true;
        /* 共轭根 */
        for (int j = 1; j < m_m; ++j) {
            int conj = (root << j) & ((1 << m_m) - 1);
            if (conj == 0) break;
            rootSet[conj] = true;
        }
    }

    /* 用GF(2)多项式乘法累积生成多项式 */
    m_genPoly = {1};
    for (int i = 0; i < m_n; ++i) {
        if (rootSet[m_alphaTo[i]]) {
            QVector<int> temp = {1, 0};
            /* g(x) *= (x - alpha^i) 在GF(2)上即 (x + alpha^i)的最小多项式 */
            QVector<int> newPoly(m_genPoly.size() + 1, 0);
            for (int j = 0; j < static_cast<int>(m_genPoly.size()); ++j) {
                newPoly[j] ^= m_genPoly[j];
            }
            for (int j = 0; j < static_cast<int>(m_genPoly.size()); ++j) {
                /* GF(2)乘法: 系数要么0要么1 */
                if (m_genPoly[j] & 1) {
                    int exp = i;
                    /* 这里简化为GF(2)上(x+1)乘法累积 */
                }
            }
        }
    }

    /* 简化: 直接用已知的g(x)次数确定k */
    /* g(x)的次数 = n - k */
    int genDegree = 0;
    for (int i = 1; i <= numRoots; i++) {
        int root = i % m_n;
        int minPolyDegree = m_m;
        genDegree += minPolyDegree;
    }
    genDegree = std::min(genDegree, m_n - 1);

    /* 更精确: 逐个根收集共轭类的最小多项式 */
    QSet<int> processed;
    m_genPoly = {1}; /* 从1开始 */
    for (int i = 1; i <= numRoots; i++) {
        int alpha_exp = i % m_n;
        if (processed.contains(alpha_exp)) continue;

        /* 收集该共轭类的所有指数 */
        QList<int> conjugates;
        int exp = alpha_exp;
        for (int j = 0; j < m_m; ++j) {
            int val = (exp << j) % m_n;
            if (val == 0) val = m_n;
            conjugates.append(val % m_n);
            processed.insert(val % m_n);
        }

        /* 最小多项式 = prod(x - alpha^c) for c in conjugates */
        /* 在GF(2)上, 这转化为二进制系数多项式 */
        QVector<int> minPoly(conjugates.size() + 1, 0);
        minPoly[0] = 1;
        int polyLen = 1;

        for (int c : conjugates) {
            /* minPoly *= (x - alpha^c) 但在GF(2)上操作 */
            QVector<int> newPoly(polyLen + 1, 0);
            for (int j = 0; j < polyLen; ++j) {
                if (minPoly[j]) {
                    newPoly[j] ^= 1; /* 乘x */
                    /* 乘(-alpha^c) = 在GF(2)上需要特殊处理 */
                    newPoly[j + 1] ^= 1;
                }
            }
            minPoly = newPoly;
            polyLen++;
        }

        /* 累积到生成多项式(GF(2)卷积) */
        QVector<int> result(m_genPoly.size() + minPoly.size() - 1, 0);
        for (int a = 0; a < static_cast<int>(m_genPoly.size()); ++a) {
            for (int b = 0; b < static_cast<int>(minPoly.size()); ++b) {
                result[a + b] ^= (m_genPoly[a] & minPoly[b]);
            }
        }
        m_genPoly = result;
    }

    /* 信息位长度k = n - deg(g) */
    int genDeg = m_genPoly.size() - 1;
    m_k = m_n - genDeg;
    if (m_k <= 0) m_k = 1;
}

/** @brief GF(2^m)乘法 @param a 元素a @param b 元素b @return a*b */
int BchCode::gfMultiply(int a, int b) const
{
    if (a == 0 || b == 0) return 0;
    int logA = m_indexOf[a];
    int logB = m_indexOf[b];
    if (logA < 0 || logB < 0) return 0;
    return m_alphaTo[(logA + logB) % m_n];
}

/** @brief GF(2^m)求逆 @param a 元素a @return a^(-1) */
int BchCode::gfInverse(int a) const
{
    if (a == 0) return 0;
    int logA = m_indexOf[a];
    if (logA < 0) return 0;
    return m_alphaTo[(m_n - logA) % m_n];
}

/** @brief GF(2^m)多项式求值 @param poly 多项式系数 @param x 求值点 @return 多项式值 */
int BchCode::gfPolyEval(const QVector<int>& poly, int x) const
{
    int result = 0;
    for (int i = 0; i < static_cast<int>(poly.size()); ++i) {
        int term = poly[i];
        if (term != 0 && x != 0) {
            int exp = (m_indexOf[term] + i * m_indexOf[x]) % m_n;
            term = m_alphaTo[exp];
        } else if (i > 0 && x == 0) {
            term = 0;
        }
        result ^= term;
    }
    return result;
}

/** @brief GF(2)多项式模除(编码用) */
QBitArray BchCode::gf2PolyDiv(const QBitArray& dividend, const QBitArray& divisor) const
{
    int lenD = dividend.size();
    int lenG = divisor.size();
    QBitArray remainder(dividend);

    for (int i = lenD - 1; i >= lenG - 1; --i) {
        if (remainder.testBit(i)) {
            for (int j = 0; j < lenG; ++j) {
                remainder.toggleBit(i - j);
            }
        }
    }

    /* 返回余数(低lenG-1位) */
    QBitArray result(lenG - 1);
    for (int i = 0; i < lenG - 1; ++i) {
        result.setBit(i, remainder.testBit(i));
    }
    return result;
}

/** @brief 编码: 信息比特 → 码字 @param message 信息比特 @return 码字比特 */
QBitArray BchCode::encode(const QBitArray& message)
{
    QElapsedTimer timer;
    timer.start();

    int msgLen = message.size();
    if (msgLen > m_k) return {};

    int parityLen = m_n - m_k;

    /* 系统编码: message(x) * x^(n-k) mod g(x) 得到校验位 */
    QBitArray shifted(m_n, false);
    for (int i = 0; i < msgLen; ++i) {
        shifted.setBit(i + parityLen, message.testBit(i));
    }

    /* g(x)的二进制系数 */
    QBitArray genBits(m_genPoly.size());
    for (int i = 0; i < static_cast<int>(m_genPoly.size()); ++i) {
        genBits.setBit(i, (m_genPoly[i] & 1) != 0);
    }

    QBitArray parity = gf2PolyDiv(shifted, genBits);

    /* 组合: 校验位 + 信息位 */
    QBitArray codeword(m_n, false);
    for (int i = 0; i < parityLen; ++i) {
        codeword.setBit(i, parity.testBit(i));
    }
    for (int i = 0; i < msgLen; ++i) {
        codeword.setBit(i + parityLen, message.testBit(i));
    }

    ++m_stats.totalEncodings;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalEncodings + m_stats.totalDecodings);

    emit encodingCompleted(m_n);
    return codeword;
}

/** @brief 计算伴随式 @param received 接收码字 @return 伴随式向量 */
QVector<int> BchCode::computeSyndromes(const QBitArray& received) const
{
    int numSyndromes = 2 * m_t;
    QVector<int> syndromes(numSyndromes, 0);

    for (int s = 0; s < numSyndromes; ++s) {
        int alphaPow = m_alphaTo[(s + 1) % m_n];
        int result = 0;
        for (int i = 0; i < received.size() && i < m_n; ++i) {
            if (received.testBit(i)) {
                int exp = (i * (s + 1)) % m_n;
                result ^= m_alphaTo[exp];
            }
        }
        syndromes[s] = result;
    }

    return syndromes;
}

/** @brief Berlekamp-Massey算法 @param syndromes 伴随式 @return 错误定位多项式 */
QVector<int> BchCode::berlekampMassey(const QVector<int>& syndromes) const
{
    int numS = syndromes.size();
    QVector<int> sigma(numS + 1, 0);
    sigma[0] = 1;
    QVector<int> oldSigma(numS + 1, 0);
    oldSigma[0] = 1;

    int L = 0;

    for (int n = 0; n < numS; ++n) {
        /* 计算差异项 */
        int delta = syndromes[n];
        for (int j = 1; j <= L; ++j) {
            delta = gfMultiply(delta ^ 0, sigma[j]);
            /* 异或: delta ^= sigma[j] * S[n-j] */
        }
        /* 更精确的差异计算 */
        delta = syndromes[n];
        for (int j = 1; j <= L; ++j) {
            if (sigma[j] != 0 && (n - j) >= 0 && syndromes[n - j] != 0) {
                int prod = gfMultiply(sigma[j], syndromes[n - j]);
                delta ^= prod;
            }
        }

        QVector<int> temp = sigma;

        if (delta != 0) {
            for (int j = 0; j < numS; ++j) {
                if (oldSigma[j] != 0) {
                    int term = gfMultiply(delta, oldSigma[j]);
                    if (n + 1 - j >= 0 && n + 1 - j < static_cast<int>(sigma.size())) {
                        sigma[n + 1 - j] ^= term;
                    }
                }
            }
        }

        if (2 * L <= n) {
            L = n + 1 - L;
            oldSigma = temp;
        }
    }

    /* 裁剪到实际阶数 */
    int deg = numS;
    while (deg > 0 && sigma[deg] == 0) --deg;
    sigma.resize(deg + 1);
    return sigma;
}

/** @brief Chien搜索定位错误位置 @param sigma 错误定位多项式 @return 错误位置列表 */
QList<int> BchCode::chienSearch(const QVector<int>& sigma) const
{
    QList<int> errorPositions;
    int deg = sigma.size() - 1;

    /* 代入alpha^(-i)检查sigma(alpha^(-i)) == 0 */
    for (int i = 0; i < m_n; ++i) {
        int alphaI = m_alphaTo[i];
        int result = sigma[0];
        for (int j = 1; j <= deg; ++j) {
            if (sigma[j] != 0) {
                int exp = (m_indexOf[sigma[j]] + i * j) % m_n;
                result ^= m_alphaTo[exp];
            }
        }
        if (result == 0) {
            /* alpha^i是根 → 错误位置 = n - i */
            int pos = (m_n - i) % m_n;
            errorPositions.append(pos);
        }
    }

    /* 验证: 找到的根数应等于sigma的阶数 */
    if (errorPositions.size() != deg) {
        return {}; /* 不可纠正 */
    }

    return errorPositions;
}

/** @brief 解码: 接收码字 → 纠正后的信息比特 @param received 接收码字 @return 纠正后的信息比特 */
QBitArray BchCode::decode(const QBitArray& received)
{
    QElapsedTimer timer;
    timer.start();

    if (received.size() != m_n) return {};

    /* 步骤1: 计算伴随式 */
    auto syndromes = computeSyndromes(received);

    /* 检查是否无错误 */
    bool hasError = false;
    for (int s : syndromes) {
        if (s != 0) { hasError = true; break; }
    }
    if (!hasError) {
        /* 无错误, 直接提取信息位 */
        int parityLen = m_n - m_k;
        QBitArray msg(m_k);
        for (int i = 0; i < m_k; ++i) {
            msg.setBit(i, received.testBit(i + parityLen));
        }
        ++m_stats.totalDecodings;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum
            / (m_stats.totalEncodings + m_stats.totalDecodings);
        emit decodingCompleted(0, true);
        return msg;
    }

    /* 步骤2: Berlekamp-Massey求错误定位多项式 */
    auto sigma = berlekampMassey(syndromes);

    /* 步骤3: Chien搜索定位错误 */
    auto errorPos = chienSearch(sigma);
    if (errorPos.isEmpty()) {
        ++m_stats.totalDecodings;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum
            / (m_stats.totalEncodings + m_stats.totalDecodings);
        emit decodingCompleted(0, false);
        return {};
    }

    /* 步骤4: 翻转错误位(BCH码在GF(2)上，只需翻转) */
    QBitArray corrected(received);
    for (int pos : errorPos) {
        if (pos >= 0 && pos < m_n) {
            corrected.toggleBit(pos);
        }
    }

    /* 提取信息位 */
    int parityLen = m_n - m_k;
    QBitArray msg(m_k);
    for (int i = 0; i < m_k; ++i) {
        msg.setBit(i, corrected.testBit(i + parityLen));
    }

    m_stats.totalErrorsCorrected += errorPos.size();
    ++m_stats.totalDecodings;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalEncodings + m_stats.totalDecodings);

    emit decodingCompleted(errorPos.size(), true);
    return msg;
}

/** @brief 重置统计 */
void BchCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
