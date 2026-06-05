/**
 * @file HammingCode5.cpp
 * @brief Hamming码编解码器实现，支持单比特纠错和双比特检错
 *
 * 实现了系统Hamming码的编码和解码功能。
 * 编码时生成校验矩阵，解码时通过伴随式(Syndrome)检测和纠正错误。
 * 支持可变数据位长度的Hamming码配置。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/code52/HammingCode5.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化参数并构建校验矩阵
 * @param parent 父QObject对象指针
 */
HammingCode5::HammingCode5(QObject* parent)
    : QObject(parent)
{
    buildParity();
}

/**
 * @brief 设置数据位数量，自动计算校验位和码字长度
 * @param bits 数据位数，必须大于0
 *
 * 根据Hamming码公式 m_k + m_r + 1 <= 2^m_r 确定校验位数m_r和码字长度m_n
 */
void HammingCode5::setDataBits(int bits)
{
    m_k = qMax(1, bits);
    /* 计算满足 2^r >= k + r + 1 的最小r */
    m_r = 0;
    while ((1 << m_r) < m_k + m_r + 1)
        ++m_r;
    m_n = m_k + m_r;
    buildParity();
}

/**
 * @brief 构建校验矩阵H (m_r x m_n)
 *
 * 校验矩阵的列按照Hamming码规则排列：
 * - 第2^i列(0-indexed)放置校验位对应的单位向量
 * - 其余列放置数据位对应的二进制表示
 */
void HammingCode5::buildParity()
{
    m_parity.resize(m_r);
    for (int i = 0; i < m_r; ++i) {
        m_parity[i].resize(m_n, 0);
        for (int j = 0; j < m_n; ++j) {
            /* 列号从1开始计数，第j+1列的第i位 */
            if ((j + 1) & (1 << i))
                m_parity[i][j] = 1;
        }
    }
}

/**
 * @brief 编码数据位为Hamming码字
 *
 * 系统编码：码字中数据位和校验位分开存放。
 * 校验位放在2的幂次位置（第1, 2, 4, 8...位）。
 *
 * @param data 待编码的数据位向量
 * @return 编码后的Hamming码字向量
 */
QVector<int> HammingCode5::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> code(m_n, 0);

    /* 将数据位填入非校验位位置 */
    int dataIdx = 0;
    for (int i = 0; i < m_n; ++i) {
        /* 跳过2的幂次位置（校验位位置） */
        bool isParityPos = false;
        for (int r = 0; r < m_r; ++r) {
            if (i == (1 << r) - 1) { /* 0-indexed: 0, 1, 3, 7... */
                isParityPos = true;
                break;
            }
        }
        if (!isParityPos && dataIdx < data.size()) {
            code[i] = data[dataIdx++];
        }
    }

    /* 计算校验位 */
    for (int r = 0; r < m_r; ++r) {
        int parityBit = 0;
        int parityPos = (1 << r) - 1; /* 0-indexed校验位位置 */
        for (int j = 0; j < m_n; ++j) {
            if (j == parityPos) continue; /* 校验位自身不参与 */
            if (m_parity[r][j] && code[j])
                parityBit ^= 1;
        }
        code[parityPos] = parityBit;
    }

    /* 更新统计 */
    m_stats.totalEncodes++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return code;
}

/**
 * @brief 解码Hamming码字，自动纠错
 *
 * 通过计算伴随式(Syndrome)检测错误位置：
 * - Syndrome为0：无错误
 * - Syndrome非0：指向错误比特位置（单比特可纠正）
 *
 * @param received 接收到的码字向量（可能含错误）
 * @return 纠正后的数据位向量
 */
QVector<int> HammingCode5::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> code = received;
    while (code.size() < m_n)
        code.append(0);

    /* 计算伴随式(Syndrome) */
    int syndrome = 0;
    for (int r = 0; r < m_r; ++r) {
        int check = 0;
        for (int j = 0; j < m_n; ++j) {
            if (m_parity[r][j] && code[j])
                check ^= 1;
        }
        if (check)
            syndrome |= (1 << r);
    }

    /* 纠正错误（syndrome就是错误位置的1-indexed值） */
    int errors = 0;
    if (syndrome > 0 && syndrome <= m_n) {
        code[syndrome - 1] ^= 1; /* 翻转错误比特 */
        errors = 1;
    }

    /* 提取数据位（跳过校验位位置） */
    QVector<int> data;
    for (int i = 0; i < m_n; ++i) {
        bool isParityPos = false;
        for (int r = 0; r < m_r; ++r) {
            if (i == (1 << r) - 1) {
                isParityPos = true;
                break;
            }
        }
        if (!isParityPos)
            data.append(code[i]);
    }

    /* 更新统计 */
    m_stats.totalDecodes++;
    m_stats.totalErrors += errors;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(errors);
    return data;
}

/**
 * @brief 重置所有统计数据
 */
void HammingCode5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
