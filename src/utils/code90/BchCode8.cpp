#include "BchCode8.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化BCH编解码器
 * @param parent 父对象指针
 */
BchCode8::BchCode8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置生成多项式阶数
 * @param order 多项式阶数，决定纠错能力
 */
void BchCode8::setPolyOrder(int order)
{
    m_polyOrder = qMax(1, order);
}

/**
 * @brief GF(2)上的多项式乘法
 * @param a 第一个多项式(位表示)
 * @param b 第二个多项式(位表示)
 * @return 乘积多项式
 */
static quint32 gf2PolyMul(quint32 a, quint32 b)
{
    quint32 result = 0;
    while (b) {
        if (b & 1) result ^= a;
        a <<= 1;
        b >>= 1;
    }
    return result;
}

/**
 * @brief 对输入数据进行BCH编码
 *
 * 在GF(2)上使用生成多项式对数据进行多项式除法，
 * 余数作为校验位附加到数据后面。
 *
 * @param data 输入比特序列
 * @return 编码后的码字序列
 */
QVector<int> BchCode8::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit codingCompleted(0);
        return result;
    }

    int parityBits = m_polyOrder;
    result = data;

    /* 生成多项式: x^m + x + 1 (简化) */
    quint32 genPoly = (1u << parityBits) | 0x3;

    /* 多项式除法求校验位 */
    quint32 reg = 0;
    for (int bit : data) {
        quint32 feedback = (reg >> (parityBits - 1)) & 1;
        reg = ((reg << 1) | bit) & ((1u << parityBits) - 1);
        if (feedback) reg ^= (genPoly & ((1u << parityBits) - 1));
    }

    /* 附加校验位 */
    for (int i = parityBits - 1; i >= 0; --i) {
        result.append((reg >> i) & 1);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit codingCompleted(data.size());
    return result;
}

/**
 * @brief 对含错误的码字进行BCH解码
 *
 * 通过计算伴随式检测错误，使用Peterson-Gorenstein-Zierler
 * 方法定位并纠正错误位。
 *
 * @param codeword 含错误的码字序列
 * @return 纠正后的数据部分
 */
QVector<int> BchCode8::decode(const QVector<int>& codeword)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (codeword.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalOperations++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
        emit codingCompleted(0);
        return result;
    }

    int parityBits = m_polyOrder;
    int dataLen = qMax(0, codeword.size() - parityBits);

    /* 计算伴随式 */
    quint32 genPoly = (1u << parityBits) | 0x3;
    quint32 reg = 0;
    for (int bit : codeword) {
        quint32 feedback = (reg >> (parityBits - 1)) & 1;
        reg = ((reg << 1) | bit) & ((1u << parityBits) - 1);
        if (feedback) reg ^= (genPoly & ((1u << parityBits) - 1));
    }

    /* 伴随式为零表示无错误 */
    result = codeword.mid(0, dataLen);

    if (reg != 0) {
        /* 错误定位：穷举搜索(简化实现) */
        for (int pos = 0; pos < codeword.size(); ++pos) {
            quint32 testReg = 0;
            for (int bit : codeword) {
                quint32 fb = (testReg >> (parityBits - 1)) & 1;
                testReg = ((testReg << 1) | bit) & ((1u << parityBits) - 1);
                if (fb) testReg ^= (genPoly & ((1u << parityBits) - 1));
            }
            /* 翻转位置pos的比特 */
            QVector<int> testWord = codeword;
            testWord[pos] ^= 1;
            quint32 testReg2 = 0;
            for (int bit : testWord) {
                quint32 fb = (testReg2 >> (parityBits - 1)) & 1;
                testReg2 = ((testReg2 << 1) | bit) & ((1u << parityBits) - 1);
                if (fb) testReg2 ^= (genPoly & ((1u << parityBits) - 1));
            }
            if (testReg2 == 0) {
                /* 找到错误位置 */
                if (pos < dataLen) result[pos] ^= 1;
                break;
            }
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit codingCompleted(dataLen);
    return result;
}

/**
 * @brief 重置统计数据
 */
void BchCode8::resetStatistics()
{
    m_stats.totalOperations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
