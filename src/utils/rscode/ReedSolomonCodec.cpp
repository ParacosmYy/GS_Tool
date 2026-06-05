/**
 * @file ReedSolomonCodec.cpp
 * @brief Reed-Solomon编解码器实现 — GF(2^8)/Berlekamp-Massey
 */

#include "utils/rscode/ReedSolomonCodec.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * GF(2^8)本原多项式: x^8 + x^4 + x^3 + x^2 + 1 = 0x11d
 */
static constexpr quint32 GF_POLY = 0x11d;

/** @brief 构造函数 @param nsym 校验符号数 @param parent 父对象 */
ReedSolomonCodec::ReedSolomonCodec(int nsym, QObject* parent)
    : QObject(parent)
    , m_nsym(nsym > 0 ? nsym : 10)
    , m_expTable(512, 0)
    , m_logTable(256, 0)
    , m_timeSum(0.0)
{
    /* 生成GF(2^8)指数/对数表 */
    quint32 x = 1;
    for (quint32 i = 0; i < 255; ++i) {
        m_expTable[i] = static_cast<quint8>(x);
        m_logTable[static_cast<quint8>(x)] = static_cast<quint8>(i);
        x <<= 1;
        if (x & 0x100) x ^= GF_POLY;
    }
    /* 扩展exp表以简化乘法运算 */
    for (quint32 i = 255; i < 512; ++i) {
        m_expTable[i] = m_expTable[i - 255];
    }
}

/** @brief GF乘法 @param a @param b @return a*b in GF */
quint8 ReedSolomonCodec::gfMul(quint8 a, quint8 b) const
{
    if (a == 0 || b == 0) return 0;
    return m_expTable[m_logTable[a] + m_logTable[b]];
}

/** @brief GF除法 @param a @param b @return a/b in GF */
quint8 ReedSolomonCodec::gfDiv(quint8 a, quint8 b) const
{
    if (a == 0) return 0;
    if (b == 0) return 0; /* 除零保护 */
    return m_expTable[(m_logTable[a] + 255 - m_logTable[b]) % 255];
}

/** @brief GF幂 @param a 底 @param n 指数 @return a^n */
quint8 ReedSolomonCodec::gfPow(quint8 a, int n) const
{
    if (a == 0) return (n == 0) ? 1 : 0;
    int r = (static_cast<int>(m_logTable[a]) * n) % 255;
    if (r < 0) r += 255;
    return m_expTable[r];
}

/** @brief GF逆元 @param a @return a^-1 */
quint8 ReedSolomonCodec::gfInverse(quint8 a) const
{
    if (a == 0) return 0;
    return m_expTable[255 - m_logTable[a]];
}

/** @brief 多项式求值 @param poly 多项式 @param x 自变量 @return poly(x) */
quint8 ReedSolomonCodec::gfPolyEval(const QVector<quint8>& poly, quint8 x) const
{
    quint8 result = 0;
    for (int i = 0; i < poly.size(); ++i) {
        quint8 term = gfMul(poly[i], gfPow(x, poly.size() - 1 - i));
        result = result ^ term;
    }
    return result;
}

/** @brief 多项式乘法 @param a @param b @return a*b */
QVector<quint8> ReedSolomonCodec::gfPolyMul(
    const QVector<quint8>& a, const QVector<quint8>& b) const
{
    if (a.isEmpty() || b.isEmpty()) return {};

    QVector<quint8> result(a.size() + b.size() - 1, 0);
    for (int i = 0; i < a.size(); ++i) {
        for (int j = 0; j < b.size(); ++j) {
            result[i + j] ^= gfMul(a[i], b[j]);
        }
    }
    return result;
}

/** @brief 多项式加法 @param a @param b @return a+b */
QVector<quint8> ReedSolomonCodec::gfPolyAdd(
    const QVector<quint8>& a, const QVector<quint8>& b) const
{
    int maxLen = std::max(a.size(), b.size());
    QVector<quint8> result(maxLen, 0);
    for (int i = 0; i < a.size(); ++i) {
        result[maxLen - a.size() + i] = a[i];
    }
    for (int i = 0; i < b.size(); ++i) {
        result[maxLen - b.size() + i] ^= b[i];
    }
    return result;
}

/** @brief 多项式缩放 @param p 多项式 @param x 标量 @return x*p */
QVector<quint8> ReedSolomonCodec::gfPolyScale(
    const QVector<quint8>& p, quint8 x) const
{
    QVector<quint8> result(p.size());
    for (int i = 0; i < p.size(); ++i) {
        result[i] = gfMul(p[i], x);
    }
    return result;
}

/** @brief 生成多项式 g(x) = (x-a^0)(x-a^1)...(x-a^(nsym-1)) */
QVector<quint8> ReedSolomonCodec::generatorPoly() const
{
    QVector<quint8> g = {1};
    for (int i = 0; i < m_nsym; ++i) {
        QVector<quint8> term = {1, gfPow(2, i)};
        g = gfPolyMul(g, term);
    }
    return g;
}

/** @brief 编码 @param data 消息 @return 码字(消息+校验) */
QVector<quint8> ReedSolomonCodec::encode(const QVector<quint8>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return {};

    QVector<quint8> gen = generatorPoly();

    /* 消息多项式乘以x^nsym(左移nsym位) */
    QVector<quint8> shifted(data.size() + m_nsym, 0);
    for (int i = 0; i < data.size(); ++i) {
        shifted[i] = data[i];
    }

    /* 多项式长除法取余数 */
    for (int i = 0; i < data.size(); ++i) {
        if (shifted[i] != 0) {
            for (int j = 1; j < gen.size(); ++j) {
                shifted[i + j] ^= gfMul(gen[j], shifted[i]);
            }
        }
    }

    /* 码字 = 消息 + 校验 */
    QVector<quint8> codeword(data.size() + m_nsym);
    for (int i = 0; i < data.size(); ++i) {
        codeword[i] = data[i];
    }
    for (int i = 0; i < m_nsym; ++i) {
        codeword[data.size() + i] = shifted[data.size() + i];
    }

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit encoded(data.size(), codeword.size());
    return codeword;
}

/** @brief 解码 @param data 接收数据 @return 纠正后消息 */
QVector<quint8> ReedSolomonCodec::decode(const QVector<quint8>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n <= m_nsym) {
        m_stats.totalDecodes++;
        m_stats.totalFailures++;
        emit decodeFailed();
        return {};
    }

    /* 计算校验子 */
    QVector<quint8> syndromes(m_nsym, 0);
    bool hasError = false;
    for (int i = 0; i < m_nsym; ++i) {
        quint8 s = 0;
        for (int j = 0; j < n; ++j) {
            s ^= gfMul(data[j], gfPow(2, i * (n - 1 - j)));
        }
        syndromes[i] = s;
        if (s != 0) hasError = true;
    }

    if (!hasError) {
        /* 无错误 */
        QVector<quint8> result(n - m_nsym);
        for (int i = 0; i < result.size(); ++i) {
            result[i] = data[i];
        }
        m_stats.totalDecodes++;
        m_timeSum += timer.elapsed();
        quint64 total = m_stats.totalEncodes + m_stats.totalDecodes;
        m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
        emit decoded(0);
        return result;
    }

    /* Berlekamp-Massey算法 */
    QVector<quint8> sigma = berlekampMassey(syndromes);

    /* Chien搜索 */
    QVector<int> positions = chienSearch(sigma, n);

    if (positions.isEmpty()) {
        m_stats.totalDecodes++;
        m_stats.totalFailures++;
        m_timeSum += timer.elapsed();
        emit decodeFailed();
        return {};
    }

    /* Forney算法求错误值 */
    QVector<quint8> errValues = forneyAlgorithm(sigma, syndromes, positions);

    /* 纠正错误 */
    QVector<quint8> corrected = data;
    int correctedCount = 0;
    for (int i = 0; i < positions.size() && i < errValues.size(); ++i) {
        int pos = positions[i];
        if (pos >= 0 && pos < n) {
            corrected[pos] ^= errValues[i];
            correctedCount++;
        }
    }

    m_stats.totalDecodes++;
    m_stats.totalCorrections += correctedCount;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    /* 返回消息部分 */
    QVector<quint8> result(n - m_nsym);
    for (int i = 0; i < result.size(); ++i) {
        result[i] = corrected[i];
    }

    emit decoded(correctedCount);
    return result;
}

/** @brief 计算校验子 @param data 数据 @return 校验子 */
QVector<quint8> ReedSolomonCodec::calculateSyndromes(const QVector<quint8>& data)
{
    int n = data.size();
    QVector<quint8> syndromes(m_nsym, 0);
    for (int i = 0; i < m_nsym; ++i) {
        quint8 s = 0;
        for (int j = 0; j < n; ++j) {
            s ^= gfMul(data[j], gfPow(2, i * (n - 1 - j)));
        }
        syndromes[i] = s;
    }
    return syndromes;
}

/** @brief 纠错能力 @return 最大可纠正符号数 */
int ReedSolomonCodec::errorCorrectionCapacity() const { return m_nsym / 2; }

/** @brief 码字长度 @param k 消息长度 @return 码字长度 */
int ReedSolomonCodec::codewordLength(int k) const { return k + m_nsym; }

/** @brief Berlekamp-Massey算法 @param syndromes 校验子 @return 错误定位多项式 */
QVector<quint8> ReedSolomonCodec::berlekampMassey(const QVector<quint8>& syndromes)
{
    QVector<quint8> C = {1};          /* 当前连接多项式 */
    QVector<quint8> B = {1};          /* 前一个连接多项式 */
    int L = 0;                        /* LFSR长度 */
    int m = 1;                        /* 步数偏移 */
    quint8 b = 1;                     /* 前一次差异 */

    for (int n = 0; n < syndromes.size(); ++n) {
        /* 计算差异 */
        quint8 d = syndromes[n];
        for (int i = 1; i <= L && i < C.size(); ++i) {
            d ^= gfMul(C[i], syndromes[n - i]);
        }

        if (d == 0) {
            ++m;
        } else if (2 * L <= n) {
            QVector<quint8> T = C;
            quint8 coeff = gfDiv(d, b);
            QVector<quint8> shifted(m + 1, 0);
            shifted[0] = coeff;
            /* shifted = coeff * x^m */
            /* C = C - d/b * x^m * B */
            while (C.size() < B.size() + m) C.append(0);
            for (int i = 0; i < B.size(); ++i) {
                C[m + i] ^= gfMul(coeff, B[i]);
            }
            B = T;
            L = n + 1 - L;
            b = d;
            m = 1;
        } else {
            quint8 coeff = gfDiv(d, b);
            while (C.size() < B.size() + m) C.append(0);
            for (int i = 0; i < B.size(); ++i) {
                C[m + i] ^= gfMul(coeff, B[i]);
            }
            ++m;
        }
    }

    return C;
}

/** @brief Chien搜索 @param sigma 错误定位多项式 @param n 数据长度 @return 错误位置 */
QVector<int> ReedSolomonCodec::chienSearch(const QVector<quint8>& sigma, int n)
{
    QVector<int> positions;
    int deg = sigma.size() - 1;

    for (int i = 0; i < n; ++i) {
        quint8 val = gfPolyEval(sigma, gfPow(2, i));
        if (val == 0) {
            /* 根alpha^i对应位置n-1-i */
            positions.append(n - 1 - i);
        }
    }

    return positions;
}

/** @brief Forney算法 @param sigma 定位多项式 @param syndromes 校验子 @param positions 位置 @return 错误值 */
QVector<quint8> ReedSolomonCodec::forneyAlgorithm(
    const QVector<quint8>& sigma,
    const QVector<quint8>& syndromes,
    const QVector<int>& positions)
{
    QVector<quint8> errValues;
    int deg = sigma.size() - 1;
    if (deg == 0) return errValues;

    /* 计算错误评估多项式 Omega = S * sigma mod x^nsym */
    int nsym = syndromes.size();
    QVector<quint8> omega = gfPolyMul(syndromes, sigma);
    if (omega.size() > nsym) omega.resize(nsym);

    /* 计算sigma的导数 */
    QVector<quint8> sigmaDeriv(deg, 0);
    for (int i = 0; i < deg; ++i) {
        if ((deg - 1 - i) % 2 == 0) {
            sigmaDeriv[i] = sigma[i];
        }
    }

    for (int pos : positions) {
        quint8 Xi = gfPow(2, pos);
        quint8 XiInv = gfInverse(Xi);

        quint8 sigmaPrimeVal = gfPolyEval(sigmaDeriv, XiInv);
        if (sigmaPrimeVal == 0) {
            errValues.append(0);
            continue;
        }

        quint8 omegaVal = gfPolyEval(omega, XiInv);
        quint8 e = gfDiv(omegaVal, sigmaPrimeVal);
        errValues.append(e);
    }

    return errValues;
}

/** @brief 重置统计 */
void ReedSolomonCodec::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
