/**
 * @file BchCode5.cpp
 * @brief BCH纠错码编解码器实现 — 有限域GF(2^m)上的循环码
 *
 * 基于BCH码理论实现编码与Berlekamp-Massey迭代译码，
 * 支持可配置的有限域阶数和设计距离。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/code62/BchCode5.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化参数并计算生成多项式
 * @param parent 父QObject指针
 */
BchCode5::BchCode5(QObject* parent)
    : QObject(parent)
{
    computeGeneratorPoly();
}

/**
 * @brief 设置有限域阶数 GF(2^m)
 * @param m 有限域阶数参数，必须 >= 2
 */
void BchCode5::setFieldOrder(int m)
{
    m_m = qMax(2, m);
    m_n = (1 << m_m) - 1;  /* 码长 n = 2^m - 1 */
    computeGeneratorPoly();
}

/**
 * @brief 设置设计距离
 *
 * 纠错能力 t = floor((d-1)/2)，信息位长度 k = n - deg(g(x))
 *
 * @param d 设计距离，必须 >= 3
 */
void BchCode5::setDesiredDistance(int d)
{
    m_t = qMax(1, (d - 1) / 2);
    computeGeneratorPoly();
}

/**
 * @brief BCH码编码
 *
 * 使用生成多项式进行系统编码:
 * 1. 将信息多项式左移 (n-k) 位
 * 2. 用生成多项式除，取余数作为校验位
 * 3. 拼接信息位和校验位
 *
 * @param data 信息位序列
 * @return 编码后的码字序列
 */
QVector<int> BchCode5::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() != m_k) {
        /* 如果长度不匹配k，使用实际数据长度 */
    }

    const int dataLen = data.size();
    const int parityLen = m_n - dataLen;
    if (parityLen <= 0) {
        m_stats.totalEncodes++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
        return data;
    }

    /* 多项式除法求余数 */
    QVector<int> remainder(parityLen, 0);
    for (int i = 0; i < dataLen; ++i) {
        int feedback = data[i] ^ remainder[0];
        /* 移位 */
        for (int j = 0; j < parityLen - 1; ++j) {
            remainder[j] = remainder[j + 1];
        }
        remainder[parityLen - 1] = 0;

        /* 异或生成多项式 */
        if (feedback != 0) {
            for (int j = 0; j < m_gPoly.size() && j < parityLen; ++j) {
                remainder[parityLen - 1 - j] ^= m_gPoly[m_gPoly.size() - 1 - j];
            }
        }
    }

    /* 系统码输出: 信息位 + 校验位 */
    QVector<int> codeword;
    codeword.reserve(m_n);
    for (int i = 0; i < dataLen; ++i) {
        codeword.append(data[i]);
    }
    for (int i = 0; i < parityLen; ++i) {
        codeword.append(remainder[i]);
    }

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
    return codeword;
}

/**
 * @brief BCH码解码
 *
 * 使用Berlekamp-Massey算法和Chien搜索进行纠错译码:
 * 1. 计算伴随式(syndrome)
 * 2. BM算法求错误位置多项式
 * 3. Chien搜索定位错误位置
 * 4. 翻转错误位
 *
 * @param received 接收到的码字
 * @return 纠错后的码字
 */
QVector<int> BchCode5::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    const int len = received.size();
    if (len == 0) {
        m_stats.totalDecodes++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
        return received;
    }

    /* 步骤1: 计算伴随式 */
    QVector<int> syndrome(2 * m_t, 0);
    for (int s = 0; s < 2 * m_t; ++s) {
        int val = 0;
        for (int i = 0; i < len; ++i) {
            if (received[i]) {
                /* alpha^(i*(s+1)) 在GF(2)上的简化 */
                val ^= ((i * (s + 1)) % (m_n + 1) != 0) ? 1 : 0;
            }
        }
        syndrome[s] = val;
    }

    /* 检查是否全零(无错误) */
    bool hasError = false;
    for (int s : syndrome) {
        if (s != 0) { hasError = true; break; }
    }

    int errorCount = 0;
    bool corrected = true;
    QVector<int> decoded = received;

    if (hasError) {
        /* 步骤2: Berlekamp-Massey算法 */
        QVector<int> sigma = berlekampMassey(syndrome);

        /* 步骤3: Chien搜索 */
        QVector<int> errorPositions = chienSearch(sigma);
        errorCount = errorPositions.size();

        /* 步骤4: 翻转错误位 */
        if (errorCount <= m_t) {
            for (int pos : errorPositions) {
                if (pos >= 0 && pos < len) {
                    decoded[pos] ^= 1;
                }
            }
        } else {
            corrected = false;
        }
    }

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(errorCount, corrected);
    return decoded;
}

/**
 * @brief 重置所有统计数据
 */
void BchCode5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 计算BCH生成多项式
 *
 * 在GF(2)上求最小多项式的最小公倍式，
 * 简化实现使用预计算的校验位数。
 */
void BchCode5::computeGeneratorPoly()
{
    /* 根据m和t计算参数 */
    m_n = (1 << m_m) - 1;
    /* 简化: 校验位数 = m * t */
    int parityBits = m_m * m_t;
    m_k = m_n - parityBits;
    if (m_k < 1) m_k = 1;

    /* 构造简化的生成多项式
     * 实际BCH需要GF(2^m)上的最小多项式，这里用近似方法 */
    int gDegree = parityBits;
    m_gPoly.resize(gDegree + 1, 0);

    /* 初始化为1 */
    m_gPoly[0] = 1;

    /* 依次乘以 (x - alpha^i) 的最小多项式, i=1,3,5,...,2t-1 */
    for (int i = 0; i < m_t; ++i) {
        int rootIdx = 2 * i + 1;
        /* 每个最小多项式在GF(2)上最多m次 */
        QVector<int> minPoly(m_m + 1, 0);
        minPoly[0] = 1;
        minPoly[1] = 1; /* 简化: x + alpha^rootIdx */
        for (int j = 1; j < m_m; ++j) {
            minPoly[j + 1] = (rootIdx >> j) & 1;
        }

        /* 多项式乘法 */
        QVector<int> newPoly(gDegree + 1, 0);
        for (int a = 0; a < m_gPoly.size(); ++a) {
            for (int b = 0; b < minPoly.size(); ++b) {
                if (a + b <= gDegree) {
                    newPoly[a + b] ^= (m_gPoly[a] & minPoly[b]);
                }
            }
        }
        m_gPoly = newPoly;
    }
}

/**
 * @brief Berlekamp-Massey迭代算法
 *
 * 根据伴随式求解错误位置多项式sigma(x)。
 *
 * @param syndrome 伴随式向量
 * @return 错误位置多项式系数
 */
QVector<int> BchCode5::berlekampMassey(const QVector<int>& syndrome)
{
    const int n = syndrome.size();
    QVector<int> sigma(n + 1, 0);
    sigma[0] = 1;
    QVector<int> oldSigma(n + 1, 0);
    oldSigma[0] = 1;

    int L = 0;  /* 当前LFSR长度 */
    int m = 1;  /* 迭代偏移 */
    int b = 1;  /* 前一次失配时的系数 */

    for (int r = 0; r < n; ++r) {
        /* 计算差值 delta */
        int delta = syndrome[r];
        for (int j = 1; j <= L; ++j) {
            delta ^= (sigma[j] & syndrome[r - j]);
        }

        if (delta == 0) {
            ++m;
        } else if (2 * L <= r) {
            QVector<int> temp = sigma;
            for (int j = 0; j < n + 1 - m; ++j) {
                sigma[j + m] ^= oldSigma[j];
            }
            oldSigma = temp;
            L = r + 1 - L;
            b = delta;
            m = 1;
        } else {
            for (int j = 0; j < n + 1 - m; ++j) {
                sigma[j + m] ^= oldSigma[j];
            }
            ++m;
        }
    }

    sigma.resize(L + 1);
    return sigma;
}

/**
 * @brief Chien搜索算法
 *
 * 通过遍历有限域元素搜索错误位置多项式的根，
 * 根的倒数即为错误位置。
 *
 * @param sigma 错误位置多项式
 * @return 错误位置索引列表(0-based)
 */
QVector<int> BchCode5::chienSearch(const QVector<int>& sigma)
{
    QVector<int> positions;
    const int degree = sigma.size() - 1;
    if (degree <= 0) return positions;

    /* 在GF(2^m)上搜索sigma(alpha^i) = 0的根 */
    for (int i = 0; i < m_n; ++i) {
        int val = sigma[0];
        for (int j = 1; j <= degree; ++j) {
            if (sigma[j]) {
                /* alpha^(i*j) 的GF(2)表示 */
                val ^= ((i * j) % (m_n + 1) != 0) ? 1 : 0;
            }
        }
        if (val == 0) {
            /* 位置 = n - 1 - i (倒序映射) */
            int pos = (m_n - 1 - i + m_n) % m_n;
            positions.append(pos);
        }
    }

    return positions;
}
