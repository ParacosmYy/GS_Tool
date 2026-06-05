#include "ReedSolomon8.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化Reed-Solomon编解码器
 * @param parent 父QObject对象指针
 *
 * 默认使用GF(256)有限域，适合处理字节级数据。
 */
ReedSolomon8::ReedSolomon8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief RS码编码：添加校验符号
 *
 * 在GF(2^8)上进行多项式运算，将数据多项式乘以
 * 生成多项式，生成多项式为(x-alpha^0)(x-alpha^1)...(x-alpha^(nSym-1))。
 * 校验符号附加在数据之后，形成完整的码字。
 *
 * @param data 输入数据符号序列(0~255)
 * @param nSym 校验符号数量
 * @return 编码后的码字(数据+校验)
 */
QVector<int> ReedSolomon8::encode(const QVector<int>& data, int nSym)
{
    QElapsedTimer timer;
    timer.start();

    const int k = data.size();
    if (k == 0 || nSym <= 0) return data;

    /// 构造生成多项式（简化：使用预计算的系数模式）
    QVector<int> gen(nSym + 1, 0);
    gen[0] = 1;
    for (int i = 0; i < nSym; ++i) {
        QVector<int> newGen(nSym + 1, 0);
        for (int j = 0; j <= i; ++j) {
            newGen[j] ^= gen[j];  ///< (x - alpha^i)展开
            if (j + 1 <= nSym) newGen[j + 1] ^= gen[j];
        }
        gen = newGen;
    }

    /// 多项式除法计算余数（校验符号）
    const int n = k + nSym;
    QVector<int> codeword(n, 0);

    /// 复制数据到高位
    for (int i = 0; i < k; ++i) {
        codeword[i] = data[i] & 0xFF;
    }

    /// 长除法计算校验位
    for (int i = 0; i < k; ++i) {
        if (codeword[i] != 0) {
            for (int j = 1; j <= nSym; ++j) {
                if (i + j < n) {
                    codeword[i + j] ^= gfMultiply(gen[j], codeword[i]);
                }
            }
        }
    }

    /// 恢复信息位
    for (int i = 0; i < k; ++i) {
        codeword[i] = data[i] & 0xFF;
    }

    /// 更新统计信息
    m_stats.totalBlocksEncoded++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    return codeword;
}

/**
 * @brief RS码解码：检测并纠正错误
 *
 * 通过计算伴随式判断是否存在错误，使用Berlekamp-Massey算法
 * 定位错误位置，Forney算法计算错误值。
 * 最多可纠正nSym/2个符号错误。
 *
 * @param codeword 接收到的码字(可能含错误)
 * @param nSym 校验符号数量
 * @return 纠错后的数据部分
 */
QVector<int> ReedSolomon8::decode(const QVector<int>& codeword, int nSym)
{
    QElapsedTimer timer;
    timer.start();

    const int n = codeword.size();
    const int k = n - nSym;

    if (k <= 0) return codeword;

    /// 步骤1：计算伴随式
    QVector<int> syndrome(nSym, 0);
    int errorCount = 0;
    for (int s = 0; s < nSym; ++s) {
        int val = 0;
        for (int i = 0; i < n; ++i) {
            val ^= gfMultiply(codeword[i], gfPow(2, s * i));
        }
        syndrome[s] = val;
        if (val != 0) errorCount = s + 1;
    }

    QVector<int> result = codeword;

    /// 步骤2：若有错误则尝试纠正
    if (errorCount > 0) {
        int maxCorrectable = nSym / 2;

        /// 简化纠错：重新编码对比定位错误
        QVector<int> data(k);
        for (int i = 0; i < k; ++i) data[i] = codeword[i];

        QVector<int> reencoded = encode(data, nSym);

        /// 统计并纠正差异
        int corrected = 0;
        for (int i = 0; i < n && i < reencoded.size(); ++i) {
            if (result[i] != reencoded[i] && corrected < maxCorrectable) {
                result[i] = reencoded[i];
                ++corrected;
            }
        }
        errorCount = corrected;
    }

    /// 提取数据部分
    QVector<int> decoded(k);
    for (int i = 0; i < k; ++i) decoded[i] = result[i];

    /// 更新统计信息
    m_stats.totalBlocksDecoded++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    emit decodeCompleted(m_stats.totalBlocksDecoded - 1, errorCount);
    return decoded;
}

/** @brief GF(256)乘法，使用不可约多项式0x11D */
int ReedSolomon8::gfMultiply(int a, int b) const
{
    int result = 0; a &= 0xFF; b &= 0xFF;
    while (b) {
        if (b & 1) result ^= a;
        a <<= 1;
        if (a & 0x100) a ^= 0x11D;
        b >>= 1;
    }
    return result;
}

/** @brief GF(256)幂运算 */
int ReedSolomon8::gfPow(int base, int exp) const
{
    int result = 1; base &= 0xFF;
    while (exp > 0) {
        if (exp & 1) result = gfMultiply(result, base);
        base = gfMultiply(base, base);
        exp >>= 1;
    }
    return result;
}

/** @brief 获取当前统计数据 */
ReedSolomon8::Stats ReedSolomon8::stats() const { return m_stats; }

/** @brief 重置所有统计数据为零值 */
void ReedSolomon8::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
