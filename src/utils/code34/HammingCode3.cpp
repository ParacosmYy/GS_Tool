/**
 * @file HammingCode3.cpp
 * @brief 扩展汉明码(SEC-DED)实现 — 奇偶校验矩阵构造与伴随式解码
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 支持标准汉明码和扩展汉明码(SEC-DED)，包括：
 * - 奇偶校验矩阵自动构造
 * - 编码（数据位插入校验位）
 * - 伴随式解码（单错误纠正/双错误检测）
 */

#include "utils/code34/HammingCode3.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认参数 m=3
 * @param parent 父对象
 */
HammingCode3::HammingCode3(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("HammingCode3"));
}

/**
 * @brief 重置统计信息
 */
void HammingCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 设置汉明码参数
 *
 * 参数 m 决定码字长度：
 * - 数据位数 k = 2^m - m - 1
 * - 总位数 n = 2^m - 1
 * - 扩展模式增加 1 位总校验位
 *
 * @param m 校验位数量，有效范围 [2, 10]
 */
void HammingCode3::setParameters(int m)
{
    m_m = qBound(2, m, 10);
}

/**
 * @brief 设置是否使用扩展汉明码(SEC-DED)
 * @param ext true 启用扩展模式，额外增加一位总奇偶校验位
 */
void HammingCode3::setExtended(bool ext)
{
    m_extended = ext;
}

/**
 * @brief 获取数据位数
 * @return 数据位数 k = 2^m - m - 1
 */
int HammingCode3::dataBits() const
{
    return (1 << m_m) - m_m - 1;
}

/**
 * @brief 获取总编码位数
 * @return 标准模式返回 2^m - 1，扩展模式返回 2^m
 */
int HammingCode3::totalBits() const
{
    const int n = (1 << m_m) - 1;
    return m_extended ? n + 1 : n;
}

/**
 * @brief 计算奇偶校验矩阵中某一位位置的汉明权重（二进制中1的个数）
 * @param val 整数值
 * @return 二进制中1的个数
 */
static int popcount(int val)
{
    int count = 0;
    while (val > 0) {
        count += val & 1;
        val >>= 1;
    }
    return count;
}

/**
 * @brief 编码：将数据位插入校验位，生成汉明码字
 *
 * 编码过程：
 * 1. 校验位放在位置 1, 2, 4, 8, ... (2的幂次位置，1-indexed)
 * 2. 每个校验位覆盖其位置二进制表示中包含对应位的数据位
 * 3. 扩展模式下附加总奇偶校验位
 *
 * @param data 输入数据位序列
 * @return 编码后的码字
 */
QVector<int> HammingCode3::encode(const QVector<int> &data) const
{
    const int n = (1 << m_m) - 1;
    const int k = n - m_m;
    Q_UNUSED(k);

    // 初始化码字全为0
    QVector<int> codeword(n, 0);

    // 将数据位填入非校验位位置
    int dataIdx = 0;
    for (int pos = 1; pos <= n; ++pos) {
        // 跳过校验位位置（2的幂次）
        if ((pos & (pos - 1)) == 0) continue;
        if (dataIdx < data.size()) {
            codeword[pos - 1] = data[dataIdx] & 1;
            ++dataIdx;
        }
    }

    // 计算每个校验位
    for (int r = 0; r < m_m; ++r) {
        const int parityPos = 1 << r;
        int parity = 0;
        for (int pos = 1; pos <= n; ++pos) {
            if (pos == parityPos) continue;
            if (pos & parityPos) {
                parity ^= codeword[pos - 1];
            }
        }
        codeword[parityPos - 1] = parity;
    }

    // 扩展汉明码：附加总奇偶校验位
    if (m_extended) {
        int overallParity = 0;
        for (int i = 0; i < n; ++i) {
            overallParity ^= codeword[i];
        }
        codeword.append(overallParity);
    }

    return codeword;
}

/**
 * @brief 解码：伴随式解码，检测并纠正错误
 *
 * 解码过程：
 * 1. 计算伴随式（syndrome）：用校验矩阵的各校验位重新计算
 * 2. 伴随式非零表示存在错误，其值指向错误位置
 * 3. 扩展模式下通过总校验位区分单错和双错
 * 4. 纠正错误后提取数据位
 *
 * @param codeword 接收到的码字（可能含错误）
 * @return 纠错后的数据位序列
 */
QVector<int> HammingCode3::decode(const QVector<int> &codeword)
{
    QElapsedTimer timer;
    timer.start();

    const int n = (1 << m_m) - 1;

    // 复制码字以进行纠错
    QVector<int> corrected = codeword;
    int errorsCorrected = 0;

    if (m_extended && codeword.size() >= n + 1) {
        // 扩展汉明码的 SEC-DED 解码
        // 计算总校验位
        int overallParity = 0;
        for (int i = 0; i <= n; ++i) {
            overallParity ^= (codeword[i] & 1);
        }

        // 计算伴随式（不含总校验位）
        int syndrome = 0;
        for (int r = 0; r < m_m; ++r) {
            const int parityMask = 1 << r;
            int parity = 0;
            for (int pos = 1; pos <= n; ++pos) {
                if (pos & parityMask) {
                    parity ^= (codeword[pos - 1] & 1);
                }
            }
            if (parity != 0) {
                syndrome |= parityMask;
            }
        }

        if (syndrome == 0 && overallParity == 0) {
            // 无错误
        } else if (syndrome != 0 && overallParity != 0) {
            // 单错误（在主码字内），syndrome即为错误位置
            if (syndrome >= 1 && syndrome <= n) {
                corrected[syndrome - 1] ^= 1;
                errorsCorrected = 1;
            }
        } else if (syndrome != 0 && overallParity == 0) {
            // 双错误检测：无法纠正
            errorsCorrected = -1;
        } else {
            // syndrome == 0 且 overallParity != 0
            // 总校验位本身出错
            corrected[n] ^= 1;
            errorsCorrected = 1;
        }
    } else {
        // 标准汉明码解码
        int syndrome = 0;
        for (int r = 0; r < m_m; ++r) {
            const int parityMask = 1 << r;
            int parity = 0;
            for (int pos = 1; pos <= static_cast<int>(corrected.size()); ++pos) {
                if (pos > n) break;
                if (pos & parityMask) {
                    parity ^= (corrected[pos - 1] & 1);
                }
            }
            if (parity != 0) {
                syndrome |= parityMask;
            }
        }

        if (syndrome >= 1 && syndrome <= n) {
            corrected[syndrome - 1] ^= 1;
            errorsCorrected = 1;
        }
    }

    // 提取数据位（跳过校验位位置）
    QVector<int> data;
    for (int pos = 1; pos <= n; ++pos) {
        if ((pos & (pos - 1)) == 0) continue;
        data.append(corrected[pos - 1] & 1);
    }

    // 更新统计
    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += codeword.size();
    m_timeSum += timer.elapsed();
    const int totalOps = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = totalOps > 0 ? m_timeSum / totalOps : 0.0;

    emit decodeComplete(errorsCorrected);
    return data;
}

/**
 * @brief 检测码字中的错误数量
 *
 * 通过伴随式判断是否存在错误：
 * - syndrome == 0：无错误
 * - syndrome != 0：存在错误（可纠正为单错误）
 *
 * @param codeword 待检测码字
 * @return 0=无错误, 1=可纠正错误, -1=不可纠正(双错)
 */
int HammingCode3::detectErrors(const QVector<int> &codeword) const
{
    const int n = (1 << m_m) - 1;

    if (m_extended && codeword.size() >= n + 1) {
        int overallParity = 0;
        for (int i = 0; i <= n && i < codeword.size(); ++i) {
            overallParity ^= (codeword[i] & 1);
        }

        int syndrome = 0;
        for (int r = 0; r < m_m; ++r) {
            const int parityMask = 1 << r;
            int parity = 0;
            for (int pos = 1; pos <= n; ++pos) {
                if (pos & parityMask) {
                    parity ^= (codeword[pos - 1] & 1);
                }
            }
            if (parity != 0) syndrome |= parityMask;
        }

        if (syndrome == 0 && overallParity == 0) return 0;
        if (syndrome != 0 && overallParity != 0) return 1;
        if (syndrome != 0 && overallParity == 0) return -1;
        return 1;
    }

    int syndrome = 0;
    for (int r = 0; r < m_m; ++r) {
        const int parityMask = 1 << r;
        int parity = 0;
        for (int pos = 1; pos <= qMin(n, static_cast<int>(codeword.size())); ++pos) {
            if (pos & parityMask) {
                parity ^= (codeword[pos - 1] & 1);
            }
        }
        if (parity != 0) syndrome |= parityMask;
    }

    return (syndrome == 0) ? 0 : 1;
}
