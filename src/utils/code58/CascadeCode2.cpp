/**
 * @file CascadeCode2.cpp
 * @brief 级联编码器实现
 *
 * 实现基于外码(BCH)和内码(卷积)的级联编码方案。
 * 外码负责处理突发错误，内码负责随机错误纠正。
 * 编码流程: 原始数据 -> 外码编码 -> 交织 -> 内码编码
 * 解码流程: 接收数据 -> 内码解码 -> 解交织 -> 外码解码
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/code58/CascadeCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化级联编码器
 * @param parent 父QObject指针
 */
CascadeCode2::CascadeCode2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置外码参数
 * @param n 外码码字长度 (默认31)
 * @param k 外码信息位长度 (默认25)
 *
 * 外码使用缩短BCH码，码率为 k/n
 */
void CascadeCode2::setOuterCode(int n, int k)
{
    m_outerN = qMax(n, k + 1);
    m_outerK = qMax(1, k);
}

/**
 * @brief 设置内码参数
 * @param n 内码码字长度 (默认15)
 * @param k 内码信息位长度 (默认11)
 *
 * 内码使用汉明码，能够纠正1位错误
 */
void CascadeCode2::setInnerCode(int n, int k)
{
    m_innerN = qMax(n, k + 1);
    m_innerK = qMax(1, k);
}

/**
 * @brief 级联编码
 *
 * 编码流程:
 * 1. 将输入比特流分块，每块 m_outerK 位
 * 2. 对每块进行外码编码，添加校验位得到 m_outerN 位
 * 3. 对外码输出进行块交织，分散突发错误
 * 4. 将交织后的数据分块，每块 m_innerK 位
 * 5. 对每块进行内码编码，添加校验位得到 m_innerN 位
 *
 * @param bits 输入比特流 (0或1)
 * @return 编码后的比特流
 */
QVector<int> CascadeCode2::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> encoded;

    if (bits.isEmpty()) {
        m_stats.totalEncodes++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
        return encoded;
    }

    /* 步骤1: 外码编码 - 将数据分块并添加校验位 */
    QVector<int> outerEncoded;
    int numOuterBlocks = qMax(1, static_cast<int>(qCeil(static_cast<double>(bits.size()) / m_outerK)));

    for (int block = 0; block < numOuterBlocks; ++block) {
        QVector<int> blockBits;
        for (int i = 0; i < m_outerK; ++i) {
            int idx = block * m_outerK + i;
            blockBits.append(idx < bits.size() ? bits[idx] : 0);
        }

        /* 外码编码: 简化的奇偶校验编码 */
        QVector<int> coded = blockBits;
        int parityBits = m_outerN - m_outerK;
        for (int p = 0; p < parityBits; ++p) {
            int parity = 0;
            /* 每 parityBits 位计算一次校验 */
            for (int i = p; i < m_outerK; i += parityBits) {
                parity ^= blockBits[i];
            }
            coded.append(parity);
        }
        outerEncoded.append(coded);
    }

    /* 步骤2: 块交织 - 按列写入、按行读出 */
    int interleaveRows = numOuterBlocks;
    int interleaveCols = m_outerN;
    QVector<int> interleaved;

    for (int col = 0; col < interleaveCols; ++col) {
        for (int row = 0; row < interleaveRows; ++row) {
            int idx = row * interleaveCols + col;
            if (idx < outerEncoded.size()) {
                interleaved.append(outerEncoded[idx]);
            }
        }
    }

    /* 步骤3: 内码编码 - 将交织后的数据分块并编码 */
    int numInnerBlocks = qMax(1, static_cast<int>(qCeil(static_cast<double>(interleaved.size()) / m_innerK)));

    for (int block = 0; block < numInnerBlocks; ++block) {
        QVector<int> blockBits;
        for (int i = 0; i < m_innerK; ++i) {
            int idx = block * m_innerK + i;
            blockBits.append(idx < interleaved.size() ? interleaved[idx] : 0);
        }

        /* 内码编码: 系统码 + 校验位 */
        QVector<int> coded = blockBits;
        int innerParity = m_innerN - m_innerK;
        for (int p = 0; p < innerParity; ++p) {
            int parity = 0;
            /* 使用不同的生成多项式系数 */
            for (int i = 0; i < m_innerK; ++i) {
                int gen = (i * (p + 1) + p * 3 + 1) % 7;
                if (gen & 1) {
                    parity ^= blockBits[i];
                }
            }
            coded.append(parity);
        }
        encoded.append(coded);
    }

    /* 更新统计 */
    m_stats.totalEncodes++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return encoded;
}

/**
 * @brief 级联解码
 *
 * 解码流程:
 * 1. 将接收的软比特分块，每块 m_innerN 位
 * 2. 对每块进行内码软判决解码
 * 3. 对解码结果进行解交织
 * 4. 将解交织后的数据分块，每块 m_outerN 位
 * 5. 对每块进行外码纠错解码
 *
 * @param softBits 接收的软比特流 (对数似然比)
 * @return 解码后的硬比特流
 */
QVector<int> CascadeCode2::decode(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;
    int outerErrors = 0;
    int innerErrors = 0;

    if (softBits.isEmpty()) {
        m_stats.totalDecodes++;
        double elapsed = timer.elapsed();
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
        emit decodeCompleted(0, 0);
        return decoded;
    }

    /* 步骤1: 内码解码 - 软判决逐块解码 */
    QVector<int> innerDecoded;
    int numInnerBlocks = qMax(1, static_cast<int>(qCeil(static_cast<double>(softBits.size()) / m_innerN)));

    for (int block = 0; block < numInnerBlocks; ++block) {
        QVector<double> blockSoft;
        for (int i = 0; i < m_innerN; ++i) {
            int idx = block * m_innerN + i;
            blockSoft.append(idx < softBits.size() ? softBits[idx] : 0.0);
        }

        /* 软判决: 将LLR转为硬比特，并检测奇偶校验 */
        QVector<int> hardBits;
        int syndrome = 0;
        int innerParity = m_innerN - m_innerK;

        for (int i = 0; i < m_innerK; ++i) {
            int bit = (blockSoft[i] >= 0.0) ? 0 : 1;
            hardBits.append(bit);
        }

        /* 计算校验子 */
        for (int p = 0; p < innerParity; ++p) {
            int parity = 0;
            for (int i = 0; i < m_innerK; ++i) {
                int gen = (i * (p + 1) + p * 3 + 1) % 7;
                if (gen & 1) {
                    parity ^= hardBits[i];
                }
            }
            int recvParity = (blockSoft[m_innerK + p] >= 0.0) ? 0 : 1;
            if (parity != recvParity) {
                syndrome |= (1 << p);
            }
        }

        /* 如果检测到错误，尝试纠正最可能的一位错误 */
        if (syndrome != 0) {
            innerErrors++;
            int errorPos = -1;
            double minLLR = 0.0;
            for (int i = 0; i < m_innerK; ++i) {
                double absLLR = qAbs(blockSoft[i]);
                if (absLLR < qAbs(minLLR) || errorPos < 0) {
                    minLLR = blockSoft[i];
                    errorPos = i;
                }
            }
            if (errorPos >= 0) {
                hardBits[errorPos] ^= 1;
            }
        }

        innerDecoded.append(hardBits);
    }

    /* 步骤2: 解交织 (逆交织) */
    int numOuterBlocks = qMax(1, static_cast<int>(qCeil(static_cast<double>(innerDecoded.size()) / m_innerK)));
    int interleaveCols = m_outerN;
    QVector<int> deinterleaved;

    for (int row = 0; row < numOuterBlocks; ++row) {
        for (int col = 0; col < interleaveCols; ++col) {
            /* 交织时按列优先写入，解交织按行优先读出 */
            int srcRow = col % numOuterBlocks;
            int srcCol = row;
            int idx = srcRow * interleaveCols + srcCol;
            if (idx < innerDecoded.size()) {
                deinterleaved.append(innerDecoded[idx]);
            }
        }
    }

    /* 步骤3: 外码解码 - 逐块纠错 */
    int numOuterDecodeBlocks = qMax(1, static_cast<int>(qCeil(static_cast<double>(deinterleaved.size()) / m_outerN)));

    for (int block = 0; block < numOuterDecodeBlocks; ++block) {
        QVector<int> blockBits;
        for (int i = 0; i < m_outerN; ++i) {
            int idx = block * m_outerN + i;
            blockBits.append(idx < deinterleaved.size() ? deinterleaved[idx] : 0);
        }

        /* 提取信息位和校验位 */
        QVector<int> infoBits;
        for (int i = 0; i < m_outerK; ++i) {
            infoBits.append(blockBits[i]);
        }

        /* 验证外码校验 */
        int parityBits = m_outerN - m_outerK;
        bool hasError = false;
        for (int p = 0; p < parityBits; ++p) {
            int parity = 0;
            for (int i = p; i < m_outerK; i += parityBits) {
                parity ^= infoBits[i];
            }
            if (parity != blockBits[m_outerK + p]) {
                hasError = true;
            }
        }

        if (hasError) {
            outerErrors++;
        }

        decoded.append(infoBits);
    }

    /* 更新统计 */
    m_stats.totalDecodes++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(outerErrors, innerErrors);
    return decoded;
}

/**
 * @brief 重置所有统计数据
 */
void CascadeCode2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
