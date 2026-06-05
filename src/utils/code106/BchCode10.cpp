#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include "BchCode10.h"

namespace {
/* 文件作用域GF表和生成多项式（避免修改头文件） */
QVector<int> g_gfLog;
QVector<int> g_gfAntilog;
QVector<int> g_generator;
}

/**
 * @brief 构造函数，初始化BCH编解码器
 * @param parent 父对象指针
 */
BchCode10::BchCode10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void BchCode10::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 生成BCH码参数与生成多项式
 *
 * 根据伽罗瓦域阶数m和纠错能力t，计算码长n=2^m-1，
 * 构建GF(2^m)上的对数/反对数表，然后求出生成多项式g(x)。
 * g(x)为alpha, alpha^3, ..., alpha^(2t-1)的最小多项式之LCM。
 *
 * @param m 伽罗瓦域阶数 (3~16)
 * @param t 纠错能力 (>=1)
 * @return 参数生成是否成功
 */
bool BchCode10::generateParameters(int m, int t)
{
    if (m < 3 || m > 16 || t < 1) return false;

    m_m = m;
    m_n = (1 << m) - 1;
    m_t = t;

    /* 本原多项式系数表 (索引为m, 值为非零项的位掩码) */
    static const int primitivePoly[] = {
        0, 0, 0, 0x0B, 0x13, 0x25, 0x43, 0x89,
        0x11D, 0x211, 0x409, 0x805, 0x1053, 0x201B,
        0x402B, 0x8003, 0x1002D
    };

    int primPoly = primitivePoly[m];

    /* 构建GF(2^m)对数/反对数表 */
    g_gfLog.resize(m_n + 1);
    g_gfAntilog.resize(m_n + 1);

    int val = 1;
    for (int i = 0; i < m_n; ++i) {
        g_gfAntilog[i] = val;
        g_gfLog[val] = i;
        val <<= 1;
        if (val & (1 << m)) val ^= primPoly;
        val &= m_n;
    }
    g_gfAntilog[m_n] = 1;
    g_gfLog[0] = -1;

    /* 计算生成多项式 g(x) = LCM(M_1, M_3, ..., M_{2t-1}) */
    g_generator.fill(0, 1);
    g_generator[0] = 1;

    QVector<bool> processedRoots(m_n, false);

    for (int i = 1; i <= 2 * t; i += 2) {
        if (processedRoots[i % m_n]) continue;

        /* 收集共轭根 */
        QVector<int> roots;
        int r = i % m_n;
        for (int c = 0; c < m; ++c) {
            if (processedRoots[r]) break;
            roots.append(r);
            processedRoots[r] = true;
            r = (2 * r) % m_n;
        }
        if (roots.isEmpty()) continue;

        /* 最小多项式: 连乘(x - alpha^root) */
        QVector<int> minPoly;
        minPoly.resize(1);
        minPoly[0] = 1;

        for (int rootIdx : roots) {
            QVector<int> newPoly;
            newPoly.fill(0, minPoly.size() + 1);
            for (int j = 0; j < minPoly.size(); ++j) {
                newPoly[j + 1] ^= minPoly[j]; /* 乘x */
                if (minPoly[j] != 0) {
                    int logVal = g_gfLog[minPoly[j]];
                    int prod = g_gfAntilog[(logVal + rootIdx) % m_n];
                    newPoly[j] ^= prod;
                }
            }
            minPoly = newPoly;
        }

        /* g(x) *= minPoly (GF(2)多项式乘法) */
        QVector<int> newGen;
        newGen.fill(0, g_generator.size() + minPoly.size() - 1);
        for (int a = 0; a < g_generator.size(); ++a) {
            for (int b = 0; b < minPoly.size(); ++b) {
                newGen[a + b] ^= (g_generator[a] & minPoly[b]);
            }
        }
        g_generator = newGen;
    }

    int nMinusK = g_generator.size() - 1;
    m_k = m_n - nMinusK;
    if (m_k <= 0) return false;

    return true;
}

/**
 * @brief BCH编码
 *
 * 系统编码：将信息多项式左移(n-k)位后，用生成多项式g(x)除取余，
 * 余式附加在信息位后形成码字c(x) = m(x)*x^(n-k) + r(x)。
 *
 * @param message 信息比特序列 (长度 <= k)
 * @return 编码后的码字 (长度 = n)
 */
QVector<int> BchCode10::encode(const QVector<int>& message)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword;
    if (m_n == 0 || m_k == 0) {
        emit encodingCompleted(0);
        return codeword;
    }
    if (message.size() > m_k) {
        emit encodingCompleted(0);
        return codeword;
    }

    /* 补零到k位 */
    QVector<int> msg = message;
    while (msg.size() < m_k) msg.prepend(0);

    int nMinusK = m_n - m_k;

    /* m(x) * x^(n-k) */
    QVector<int> shifted;
    shifted.fill(0, m_n);
    for (int i = 0; i < m_k; ++i) {
        shifted[i] = msg[i];
    }

    /* 多项式除法求余式 */
    QVector<int> reg = shifted;
    for (int i = 0; i < m_k; ++i) {
        if (reg[i] != 0) {
            for (int j = 0; j < g_generator.size(); ++j) {
                reg[i + j] ^= g_generator[j];
            }
        }
    }

    /* 码字 = 信息位 + 校验位 */
    codeword.resize(m_n);
    for (int i = 0; i < m_k; ++i) codeword[i] = msg[i];
    for (int i = 0; i < nMinusK; ++i) codeword[m_k + i] = reg[m_k + i];

    m_stats.totalEncodingRuns++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncodingRuns;

    emit encodingCompleted(codeword.size());
    return codeword;
}

/**
 * @brief BCH译码 (Berlekamp-Massey + Chien搜索)
 *
 * 1. 计算伴随式 S_1...S_{2t}
 * 2. Berlekamp-Massey算法求错误定位多项式 sigma(x)
 * 3. Chien搜索查找sigma(x)的根确定错误位置
 * 4. 翻转错误位，提取信息位
 *
 * @param received 接收码字
 * @return 译码后的信息比特
 */
QVector<int> BchCode10::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;
    if (received.size() != m_n || m_n == 0) return decoded;

    /* 步骤1: 计算伴随式 */
    QVector<int> syndrome(2 * m_t + 1, 0);
    for (int s = 1; s <= 2 * m_t; ++s) {
        int val = 0;
        for (int i = 0; i < m_n; ++i) {
            if (received[i] != 0) {
                int exp = (s * i) % m_n;
                val ^= g_gfAntilog[exp];
            }
        }
        syndrome[s] = val;
    }

    /* 检查是否无错误 */
    bool hasError = false;
    for (int s = 1; s <= 2 * m_t; ++s) {
        if (syndrome[s] != 0) { hasError = true; break; }
    }
    if (!hasError) {
        decoded = received.mid(0, m_k);
        m_stats.totalErrorsCorrected += 0;
        m_stats.totalEncodingRuns++;
        qint64 elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncodingRuns;
        return decoded;
    }

    /* 步骤2: Berlekamp-Massey算法 */
    QVector<int> sigma;
    sigma.fill(0, m_t + 1);
    sigma[0] = 1;

    QVector<int> oldSigma;
    oldSigma.fill(0, m_t + 1);
    oldSigma[0] = 1;

    int L = 0;

    for (int r = 1; r <= 2 * m_t; ++r) {
        int delta = syndrome[r];
        for (int j = 1; j <= L; ++j) {
            if (sigma[j] != 0 && r - j > 0 && r - j < syndrome.size()) {
                int logS = g_gfLog[sigma[j]];
                int logSy = g_gfLog[syndrome[r - j]];
                if (logS >= 0 && logSy >= 0) {
                    delta ^= g_gfAntilog[(logS + logSy) % m_n];
                }
            }
        }

        if (delta == 0) continue;

        QVector<int> temp = sigma;
        int logDelta = g_gfLog[delta];
        for (int j = 0; j < oldSigma.size(); ++j) {
            int idx = j + 1;
            if (idx >= sigma.size()) break;
            if (oldSigma[j] == 0) continue;
            int logOld = g_gfLog[oldSigma[j]];
            int prod = g_gfAntilog[(logDelta + logOld) % m_n];
            sigma[idx] ^= prod;
        }

        if (2 * L <= r - 1) {
            L = r - L;
            oldSigma = temp;
            oldSigma.resize(m_t + 1);
        }
    }

    /* 步骤3: Chien搜索 */
    QVector<int> errorPositions;
    for (int i = 0; i < m_n; ++i) {
        int val = 0;
        for (int j = 0; j < sigma.size(); ++j) {
            if (sigma[j] != 0) {
                int logS = g_gfLog[sigma[j]];
                int exp = (logS + ((m_n - i) % m_n) * j) % m_n;
                val ^= g_gfAntilog[exp];
            }
        }
        if (val == 0) {
            errorPositions.append(i);
        }
    }

    /* 步骤4: 纠错 */
    QVector<int> corrected = received;
    int errorsFixed = qMin(errorPositions.size(), m_t);
    for (int i = 0; i < errorsFixed; ++i) {
        int pos = errorPositions[i];
        if (pos >= 0 && pos < m_n) {
            corrected[pos] ^= 1;
        }
    }

    decoded = corrected.mid(0, m_k);

    m_stats.totalErrorsCorrected += errorsFixed;
    m_stats.totalEncodingRuns++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncodingRuns;

    return decoded;
}

/**
 * @brief 获取当前码参数
 * @return [n, k, t] 码长、信息长度、纠错能力
 */
QVector<int> BchCode10::codeParameters() const
{
    return {m_n, m_k, m_t};
}
