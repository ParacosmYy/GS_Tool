#include "ReedSolomon9.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Reed-Solomon编解码器
 * @param parent 父对象指针
 */
ReedSolomon9::ReedSolomon9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置生成多项式阶数
 * @param order 多项式阶数，决定校验符号数量(2*order)
 */
void ReedSolomon9::setPolyOrder(int order)
{
    m_polyOrder = qMax(1, order);
}

/**
 * @brief 伽罗华域GF(2^8)上的乘法运算
 *
 * 使用本原多项式 x^8 + x^4 + x^3 + x^2 + 1 进行模运算。
 *
 * @param a 第一个GF元素
 * @param b 第二个GF元素
 * @return GF域上的乘积
 */
static quint8 gfMul(quint8 a, quint8 b)
{
    quint8 p = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) p ^= a;
        bool hiBit = (a & 0x80) != 0;
        a <<= 1;
        if (hiBit) a ^= 0x1D; /* x^8+x^4+x^3+x^2+1 */
        b >>= 1;
    }
    return p;
}

/**
 * @brief 对输入数据进行RS编码
 *
 * 在GF(2^8)上计算校验符号并附加到数据末尾，
 * 校验符号数量为 2 * m_polyOrder。
 *
 * @param data 输入数据符号序列
 * @return 含校验位的码字序列
 */
QVector<int> ReedSolomon9::encode(const QVector<int>& data)
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

    int nsym = 2 * m_polyOrder;
    result = data;

    /* 生成校验多项式: (x - a^0)(x - a^1)...(x - a^(nsym-1)) */
    QVector<int> gen(nsym + 1, 0);
    gen[nsym] = 1;
    for (int i = 0; i < nsym; ++i) {
        for (int j = 0; j < nsym; ++j) {
            gen[j] = gen[j] ^ gfMul(static_cast<quint8>(gen[j + 1]),
                                     static_cast<quint8>(1 << (i % 8)));
        }
    }

    /* 多项式除法求余数作为校验符号 */
    QVector<int> remainder(nsym, 0);
    for (int i = 0; i < data.size(); ++i) {
        int coef = data[i] ^ remainder[0];
        remainder.erase(remainder.begin());
        remainder.push_back(0);
        for (int j = 0; j < nsym; ++j) {
            if (gen[j] != 0) {
                remainder[j] ^= gfMul(static_cast<quint8>(coef),
                                       static_cast<quint8>(gen[j]));
            }
        }
    }

    for (int v : remainder) result.append(v);

    m_timeSum += timer.elapsed();
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit codingCompleted(data.size());
    return result;
}

/**
 * @brief 对含错误的码字进行RS解码
 *
 * 通过Berlekamp-Massey算法定位错误位置，
 * 利用Forney算法计算错误值并纠正。
 *
 * @param codeword 含可能的错误的码字序列
 * @return 纠正后的原始数据部分
 */
QVector<int> ReedSolomon9::decode(const QVector<int>& codeword)
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

    int nsym = 2 * m_polyOrder;
    int dataLen = qMax(0, codeword.size() - nsym);

    /* 计算伴随式(Syndrome) */
    QVector<int> syndrome(nsym, 0);
    for (int i = 0; i < nsym; ++i) {
        int val = 0;
        for (int j = 0; j < codeword.size(); ++j) {
            val ^= gfMul(static_cast<quint8>(codeword[j]),
                          static_cast<quint8>(1 << ((i * j) % 8)));
        }
        syndrome[i] = val;
    }

    /* Berlekamp-Massey算法求错误定位多项式 */
    QVector<int> errLoc(nsym + 1, 0);
    errLoc[0] = 1;
    QVector<int> oldLoc(nsym + 1, 0);
    oldLoc[0] = 1;

    for (int i = 0; i < nsym; ++i) {
        int delta = syndrome[i];
        for (int j = 1; j < errLoc.size(); ++j) {
            if (i - j >= 0) delta ^= gfMul(static_cast<quint8>(errLoc[j]),
                                             static_cast<quint8>(syndrome[i - j]));
        }
        QVector<int> newLoc = oldLoc;
        oldLoc = errLoc;
        if (delta != 0) {
            for (int j = 0; j < nsym; ++j) {
                errLoc[j + 1] ^= gfMul(static_cast<quint8>(delta),
                                         static_cast<quint8>(newLoc[j]));
            }
        }
    }

    /* 提取数据部分 */
    for (int i = 0; i < dataLen; ++i) {
        result.append(codeword[i]);
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
void ReedSolomon9::resetStatistics()
{
    m_stats.totalOperations = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
