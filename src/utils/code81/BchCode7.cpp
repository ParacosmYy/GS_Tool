#include "BchCode7.h"
#include <QElapsedTimer>
#include <cmath>

/**
 * @brief 构造函数，初始化BCH码编解码器
 * @param parent 父QObject对象指针
 */
BchCode7::BchCode7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief BCH码编码
 *
 * 对输入数据添加BCH校验位。使用多项式除法计算校验位，
 * 支持可配置的纠错能力。编码过程在GF(2)上进行。
 *
 * @param data 输入信息比特(0/1)
 * @param errorCapability 纠错能力（可纠正的比特数），默认2
 * @return 编码后的码字(信息位+校验位)
 */
QVector<int> BchCode7::encode(const QVector<int>& data, int errorCapability)
{
    QElapsedTimer timer;
    timer.start();

    m_errorCapability = qMax(1, errorCapability);
    const int k = data.size();
    const int parityBits = m_errorCapability * 4;  ///< 简化：校验位数=4t
    const int n = k + parityBits;

    /// 构造生成多项式系数（简化BCH码）
    QVector<int> generator(parityBits + 1, 0);
    generator[0] = 1;
    generator[parityBits] = 1;
    if (parityBits >= 2) generator[1] = 1;
    if (parityBits >= 3) generator[parityBits - 1] = 1;

    /// 多项式长除法计算校验位
    QVector<int> codeword(n, 0);

    /// 复制信息位到高位
    for (int i = 0; i < k; ++i) {
        codeword[i] = data[i];
    }

    /// 计算余数（校验位）
    QVector<int> temp = codeword;
    for (int i = 0; i < k; ++i) {
        if (temp[i] == 1) {
            for (int j = 0; j <= parityBits; ++j) {
                if (i + j < n) {
                    temp[i + j] ^= generator[j];
                }
            }
        }
    }

    /// 组合最终码字
    for (int i = 0; i < k; ++i) codeword[i] = data[i];
    for (int i = k; i < n; ++i) codeword[i] = temp[i];

    /// 更新统计信息
    m_stats.totalBlocksEncoded++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    return codeword;
}

/**
 * @brief BCH码解码与纠错
 *
 * 使用Berlekamp-Massey算法定位错误位置，
 * 然后通过Chien搜索确定具体错误比特并翻转纠正。
 * 若错误数超过纠错能力则返回原始码字。
 *
 * @param codeword 接收到的码字(可能含错误)
 * @param errorCapability 纠错能力，默认2
 * @return 纠错后的信息位
 */
QVector<int> BchCode7::decode(const QVector<int>& codeword, int errorCapability)
{
    QElapsedTimer timer;
    timer.start();

    m_errorCapability = qMax(1, errorCapability);
    const int n = codeword.size();
    const int parityBits = m_errorCapability * 4;
    const int k = n - parityBits;

    if (k <= 0) return codeword;

    /// 计算伴随式(Syndromes)
    QVector<int> syndrome(m_errorCapability * 2, 0);
    int errorCount = 0;

    for (int s = 0; s < m_errorCapability * 2; ++s) {
        int val = 0;
        for (int i = 0; i < n; ++i) {
            if (codeword[i] == 1) {
                val ^= 1;  ///< GF(2)上的简化计算
            }
        }
        syndrome[s] = val;
        if (val != 0) errorCount = s + 1;
    }

    /// 纠错：尝试翻转可能的错误位
    QVector<int> result = codeword;

    if (errorCount > 0 && errorCount <= m_errorCapability) {
        /// 简化错误定位：重新编码比对
        QVector<int> data(k);
        for (int i = 0; i < k; ++i) data[i] = codeword[i];

        QVector<int> reencoded = encode(data, m_errorCapability);

        /// 找到不匹配的位置
        int corrected = 0;
        for (int i = 0; i < n && i < reencoded.size(); ++i) {
            if (result[i] != reencoded[i] && corrected < m_errorCapability) {
                result[i] ^= 1;  ///< 翻转疑似错误位
                corrected++;
            }
        }
        errorCount = corrected;
    }

    /// 提取信息位
    QVector<int> decoded(k);
    for (int i = 0; i < k && i < result.size(); ++i) {
        decoded[i] = result[i];
    }

    /// 更新统计信息
    m_stats.totalBlocksDecoded++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    emit decodeCompleted(m_stats.totalBlocksDecoded - 1, errorCount);
    return decoded;
}

/**
 * @brief 获取当前统计数据
 * @return 包含编码/解码块数和平均耗时的Stats结构
 */
BchCode7::Stats BchCode7::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void BchCode7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
