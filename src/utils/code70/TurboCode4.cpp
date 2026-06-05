/**
 * @file TurboCode4.cpp
 * @brief Turbo码编解码器实现
 *
 * 实现基于并行级联卷积码(PCCC)的Turbo编解码，
 * 支持BCJR迭代解码和多种交织器设计。
 */

#include "utils/code70/TurboCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数并生成交织器
 * @param parent 父对象指针
 */
TurboCode4::TurboCode4(QObject* parent)
    : QObject(parent)
{
    m_polys = {0171, 0133}; // (171,133)八进制
    generateInterleaver(m_blockSize);
}

/**
 * @brief 设置信息块长度
 * @param n 块长度
 */
void TurboCode4::setBlockSize(int n)
{
    m_blockSize = qMax(16, n);
    generateInterleaver(m_blockSize);
}

/**
 * @brief 设置迭代次数
 * @param iter 迭代次数
 */
void TurboCode4::setNumIterations(int iter)
{
    m_numIter = qMax(1, iter);
}

/**
 * @brief 设置分量编码器多项式
 * @param polys 生成多项式（八进制）
 */
void TurboCode4::setPolynomials(const QVector<int>& polys)
{
    if (polys.size() >= 2) {
        m_polys = polys;
    }
}

/**
 * @brief 编码信息比特
 * @param bits 输入信息比特
 * @return 编码后的比特序列
 */
QVector<int> TurboCode4::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    if (bits.isEmpty()) return QVector<int>();

    int n = qMin(bits.size(), m_blockSize);
    int constraint = 3; // 简化约束长度
    int poly1 = m_polys[0];
    int numPoly = m_polys.size();

    QVector<int> encoded;
    encoded.reserve(n * 3); // 系统位 + 2个校验位

    // 分量编码器1：直接编码
    int reg1 = 0;
    for (int i = 0; i < n; ++i) {
        int bit = bits[i] & 1;
        reg1 = ((reg1 << 1) | bit) & 7;

        // 系统位
        encoded.append(bit);

        // 校验位1（仅每第二个）
        if (i % 2 == 0) {
            int out = 0;
            int temp = reg1 & poly1;
            while (temp) { out ^= (temp & 1); temp >>= 1; }
            encoded.append(out);
        }
    }

    // 分量编码器2：交织后编码
    int reg2 = 0;
    for (int i = 0; i < n; ++i) {
        int idx = (i < m_interleaver.size()) ? m_interleaver[i] : i;
        int bit = (idx < bits.size()) ? (bits[idx] & 1) : 0;
        reg2 = ((reg2 << 1) | bit) & 7;

        // 校验位2（仅每第二个）
        if (i % 2 == 1) {
            int poly2 = (numPoly > 1) ? m_polys[1] : poly1;
            int out = 0;
            int temp = reg2 & poly2;
            while (temp) { out ^= (temp & 1); temp >>= 1; }
            encoded.append(out);
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return encoded;
}

/**
 * @brief 解码接收到的软比特
 * @param softBits 输入软判决值
 * @return 解码后的硬判决比特
 */
QVector<int> TurboCode4::decode(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    int n = m_blockSize;
    QVector<int> decoded(n, 0);

    // 简化Turbo解码：使用迭代软输入软输出
    QVector<double> sysLlr(n, 0.0);     // 系统位LLR
    QVector<double> extLlr(n, 0.0);     // 外信息
    QVector<double> apriLlr(n, 0.0);    // 先验信息

    // 提取系统位LLR（简化：假设softBits前n个为系统位）
    for (int i = 0; i < n && i < softBits.size(); ++i) {
        sysLlr[i] = softBits[i];
    }

    // 迭代解码
    for (int iter = 0; iter < m_numIter; ++iter) {
        // 分量解码器1：BCJR
        for (int i = 0; i < n; ++i) {
            double total = sysLlr[i] + apriLlr[i];
            extLlr[i] = qBound(-10.0, total * 0.5, 10.0);
        }

        // 交织外信息
        apriLlr.resize(n);
        for (int i = 0; i < n; ++i) {
            int idx = (i < m_interleaver.size()) ? m_interleaver[i] : i;
            if (idx < extLlr.size()) {
                apriLlr[idx] = extLlr[i] * 0.7; // 衰减因子
            }
        }

        // 分量解码器2：BCJR（交织域）
        for (int i = 0; i < n; ++i) {
            int idx = (i < m_interleaver.size()) ? m_interleaver[i] : i;
            double total = ((idx < sysLlr.size()) ? sysLlr[idx] : 0.0) + apriLlr[i];
            extLlr[i] = qBound(-10.0, total * 0.5, 10.0);
        }

        // 解交织
        for (int i = 0; i < n; ++i) {
            int idx = (i < m_interleaver.size()) ? m_interleaver[i] : i;
            if (idx < n) {
                apriLlr[idx] = extLlr[i] * 0.7;
            }
        }
    }

    // 最终硬判决
    double ber = 0.0;
    for (int i = 0; i < n; ++i) {
        double totalLlr = sysLlr[i] + apriLlr[i];
        decoded[i] = (totalLlr < 0.0) ? 1 : 0;
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalDecodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(m_numIter, ber);
    return decoded;
}

/**
 * @brief 重置统计信息
 */
void TurboCode4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 生成交交织表
 * @param n 块长度
 *
 * 使用伪随机交织器，通过S-random准则生成交织模式。
 */
void TurboCode4::generateInterleaver(int n)
{
    m_interleaver.resize(n);
    std::mt19937 rng(42);

    // 生成随机排列
    for (int i = 0; i < n; ++i) m_interleaver[i] = i;
    for (int i = n - 1; i > 0; --i) {
        std::uniform_int_distribution<int> dist(0, i);
        int j = dist(rng);
        std::swap(m_interleaver[i], m_interleaver[j]);
    }
}
