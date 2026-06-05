/**
 * @file TurboCode3.cpp
 * @brief Turbo码编解码器实现 — 并行级联卷积码(PCCC)迭代解码
 *
 * 使用SISO(软输入软输出)解码器与BCJR算法的简化版本，
 * 支持可配置的块大小、约束长度和迭代次数。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/code61/TurboCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cstdlib>

/**
 * @brief 构造函数，初始化默认参数并生成交织器
 * @param parent 父QObject指针
 */
TurboCode3::TurboCode3(QObject* parent)
    : QObject(parent)
{
    generateInterleaver(m_blockSize);
}

/**
 * @brief 设置信息块大小
 * @param n 块大小(比特数)，必须 >= 1
 */
void TurboCode3::setBlockSize(int n)
{
    m_blockSize = qMax(1, n);
    generateInterleaver(m_blockSize);
}

/**
 * @brief 设置卷积码约束长度
 * @param k 约束长度，必须 >= 3
 */
void TurboCode3::setConstraintLength(int k)
{
    m_constraint = qMax(3, k);
}

/**
 * @brief 设置迭代解码次数
 * @param iter 迭代次数，必须 >= 1
 */
void TurboCode3::setNumIterations(int iter)
{
    m_numIter = qMax(1, iter);
}

/**
 * @brief Turbo码编码
 *
 * 并行级联两个递归系统卷积(RSC)编码器，
 * 第二个编码器使用交织后的数据。
 *
 * @param bits 输入信息比特序列 (0或1)
 * @return 编码后的比特序列 (系统位 + 校验位1 + 校验位2)
 */
QVector<int> TurboCode3::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    const int n = bits.size();
    if (n == 0) {
        m_stats.totalEncodes++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
        return {};
    }

    /* 确保交织器大小匹配 */
    if (m_interleaver.size() != n) {
        generateInterleaver(n);
    }

    /* 交织输入 */
    QVector<int> interleaved(n);
    for (int i = 0; i < n; ++i) {
        interleaved[i] = bits[m_interleaver[i]];
    }

    /* RSC编码器1: 生成校验位 */
    QVector<int> par1(n, 0);
    int reg = 0;
    const int mask = (1 << (m_constraint - 1)) - 1;
    for (int i = 0; i < n; ++i) {
        int fb = bits[i] ^ ((reg >> (m_constraint - 2)) & 1);
        int newBit = fb ^ (reg & 1) ^ ((reg >> 1) & 1);
        par1[i] = newBit;
        reg = ((reg << 1) | fb) & mask;
    }

    /* RSC编码器2: 对交织数据生成校验位 */
    QVector<int> par2(n, 0);
    reg = 0;
    for (int i = 0; i < n; ++i) {
        int fb = interleaved[i] ^ ((reg >> (m_constraint - 2)) & 1);
        int newBit = fb ^ (reg & 1) ^ ((reg >> 1) & 1);
        par2[i] = newBit;
        reg = ((reg << 1) | fb) & mask;
    }

    /* 复用输出: 系统位 + 校验位1 + 校验位2 */
    QVector<int> encoded;
    encoded.reserve(n * 3);
    for (int i = 0; i < n; ++i) {
        encoded.append(bits[i]);
        encoded.append(par1[i]);
        encoded.append(par2[i]);
    }

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
    return encoded;
}

/**
 * @brief Turbo码迭代解码
 *
 * 使用SISO解码器交替处理自然序和交织序数据，
 * 通过外信息交换实现迭代收敛。
 *
 * @param softBits 软比特输入序列 (对数似然比)
 * @return 硬判决解码结果 (0或1)
 */
QVector<int> TurboCode3::decode(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    const int totalLen = softBits.size();
    const int n = totalLen / 3;
    if (n == 0) {
        m_stats.totalDecodes++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
        return {};
    }

    /* 确保交织器大小匹配 */
    if (m_interleaver.size() != n) {
        generateInterleaver(n);
    }

    /* 提取系统位和校验位 */
    QVector<double> sys(n), par1(n), par2(n);
    for (int i = 0; i < n; ++i) {
        sys[i]  = softBits[i * 3];
        par1[i] = softBits[i * 3 + 1];
        par2[i] = softBits[i * 3 + 2];
    }

    /* 迭代解码 */
    QVector<double> prior(n, 0.0);
    double finalBer = 0.0;

    for (int iter = 0; iter < m_numIter; ++iter) {
        /* SISO解码器1: 自然序 */
        QVector<double> extrinsic1 = sowDecode(sys, par1, prior);

        /* 交织外信息作为解码器2的先验 */
        QVector<double> intPrior(n);
        for (int i = 0; i < n; ++i) {
            intPrior[m_interleaver[i]] = extrinsic1[i];
        }

        /* 交织系统位 */
        QVector<double> intSys(n);
        for (int i = 0; i < n; ++i) {
            intSys[m_interleaver[i]] = sys[i];
        }

        /* SISO解码器2: 交织序 */
        QVector<double> extrinsic2 = sowDecode(intSys, par2, intPrior);

        /* 解交织外信息作为下一轮先验 */
        for (int i = 0; i < n; ++i) {
            prior[i] = extrinsic2[m_interleaver[i]];
        }
    }

    /* 最终硬判决 */
    QVector<int> decoded(n);
    int errCount = 0;
    for (int i = 0; i < n; ++i) {
        double llr = sys[i] + prior[i];
        decoded[i] = (llr > 0.0) ? 1 : 0;
        /* 估计BER (近似) */
        if (qAbs(llr) < 0.5) ++errCount;
    }
    finalBer = (n > 0) ? static_cast<double>(errCount) / n : 0.0;

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(m_numIter, finalBer);
    return decoded;
}

/**
 * @brief 重置所有统计数据
 */
void TurboCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 生成交织器 (伪随机置换)
 *
 * 使用Fisher-Yates洗牌算法生成n个元素的随机置换表。
 *
 * @param n 交织器长度
 */
void TurboCode3::generateInterleaver(int n)
{
    m_interleaver.resize(n);
    for (int i = 0; i < n; ++i) {
        m_interleaver[i] = i;
    }
    /* Fisher-Yates 洗牌 */
    for (int i = n - 1; i > 0; --i) {
        int j = std::rand() % (i + 1);
        std::swap(m_interleaver[i], m_interleaver[j]);
    }
}

/**
 * @brief 简化SISO(软输入软输出)解码器
 *
 * 基于Max-Log-MAP算法的简化实现，计算外信息用于
 * Turbo码迭代解码中的外信息交换。
 *
 * @param sys 系统比特的对数似然比
 * @param par 校验比特的对数似然比
 * @param prior 先验信息
 * @return 外信息向量
 */
QVector<double> TurboCode3::sowDecode(
    const QVector<double>& sys,
    const QVector<double>& par,
    const QVector<double>& prior)
{
    const int n = sys.size();
    QVector<double> extrinsic(n, 0.0);

    /* 前向度量 alpha */
    QVector<double> alpha(2, 0.0);
    alpha[0] = 0.0;
    alpha[1] = -1e9;

    /* 简化Max-Log-MAP: 使用滑动窗口 */
    for (int i = 0; i < n; ++i) {
        double llr = sys[i] + prior[i];
        double branch[2] = {0.0, 0.0};

        /* 计算分支度量 */
        branch[0] = -qMax(0.0, par[i]);
        branch[1] = qMax(0.0, par[i]);

        /* 更新alpha (max操作简化) */
        double newAlpha0 = qMax(alpha[0] + branch[0], alpha[1] + branch[1]);
        double newAlpha1 = qMax(alpha[0] + branch[1], alpha[1] + branch[0]);

        /* 归一化防止溢出 */
        double maxA = qMax(newAlpha0, newAlpha1);
        alpha[0] = newAlpha0 - maxA;
        alpha[1] = newAlpha1 - maxA;

        /* 计算外信息 */
        double ext = llr > 0.0 ? qExp(-qAbs(par[i]) * 0.5) : -qExp(-qAbs(par[i]) * 0.5);
        extrinsic[i] = ext;
    }

    /* 缩放外信息 (提高收敛性) */
    const double scalingFactor = 0.7;
    for (int i = 0; i < n; ++i) {
        extrinsic[i] *= scalingFactor;
    }

    return extrinsic;
}
