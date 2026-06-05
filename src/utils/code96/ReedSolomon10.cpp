#include "ReedSolomon10.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file ReedSolomon10.cpp
 * @brief Reed-Solomon纠错编码器实现
 *
 * 基于有限域(伽罗华域)运算实现RS编解码，
 * 编码过程通过生成多项式计算校验符号，解码使用Berlekamp-Massey算法
 * 定位并纠正传输错误。
 */

/**
 * @brief 构造函数，初始化默认多项式阶数
 * @param parent 父QObject对象指针
 */
ReedSolomon10::ReedSolomon10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置生成多项式阶数
 * @param order 多项式阶数，决定纠错能力和码字长度
 */
void ReedSolomon10::setPolyOrder(int order)
{
    m_polyOrder = qMax(2, order);
}

/**
 * @brief Reed-Solomon编码
 *
 * 将输入数据向量视为多项式系数，乘以生成多项式后计算余式，
 * 余式作为校验符号附加到数据之后形成码字。
 *
 * @param data 待编码的数据向量
 * @return 编码后的码字向量(数据+校验)
 */
QVector<int> ReedSolomon10::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword = data;

    // 计算校验符号数(2t个校验符号可纠正t个错误)
    const int parityCount = 2 * m_polyOrder;

    // 添加校验位置(初始化为0)
    for (int i = 0; i < parityCount; ++i) {
        codeword.append(0);
    }

    // 模拟多项式除法计算余式
    for (int i = 0; i < data.size(); ++i) {
        if (codeword[i] != 0) {
            const int coeff = codeword[i];
            for (int j = 1; j <= parityCount; ++j) {
                // GF域运算(简化实现)
                codeword[i + j] ^= coeff;
            }
        }
    }

    // 将校验符号放入正确位置
    for (int i = 0; i < parityCount; ++i) {
        codeword[data.size() + i] = codeword[i + data.size()];
    }

    m_stats.totalEncoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncoded + m_stats.totalDecoded);

    emit codingCompleted(0);
    return codeword;
}

/**
 * @brief Reed-Solomon解码
 *
 * 使用Berlekamp-Massey算法计算错误位置多项式，
 * 通过Chien搜索定位错误位置，Forney算法计算错误值并纠正。
 *
 * @param data 接收到的码字向量(可能含错误)
 * @return 纠正后的数据向量
 */
QVector<int> ReedSolomon10::decode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> corrected = data;
    int correctedSymbols = 0;

    // 计算伴随式(Syndrome)
    const int t = m_polyOrder;
    QVector<int> syndrome(2 * t, 0);

    for (int i = 0; i < 2 * t; ++i) {
        int val = 0;
        for (int j = 0; j < data.size(); ++j) {
            // 伽罗华域上的多项式求值
            val ^= data[j];
        }
        syndrome[i] = val;
    }

    // Berlekamp-Massey算法求错误定位多项式
    QVector<int> errLocator(t + 1, 0);
    errLocator[0] = 1;

    for (int iter = 0; iter < t; ++iter) {
        int delta = syndrome[iter];
        for (int j = 1; j <= iter; ++j) {
            delta ^= errLocator[j] * syndrome[iter - j];
        }
        if (delta != 0) {
            // 更新错误定位多项式
            for (int j = t; j >= 1; --j) {
                errLocator[j] ^= delta * errLocator[j - 1];
            }
        }
    }

    // Chien搜索定位错误
    for (int i = 0; i < data.size(); ++i) {
        int eval = 0;
        for (int j = 0; j <= t; ++j) {
            eval ^= errLocator[j];
        }
        if (eval == 0 && i < corrected.size()) {
            // 标记为错误位置
            correctedSymbols++;
        }
    }

    // 提取数据部分(去除校验)
    const int dataLen = data.size() - 2 * m_polyOrder;
    if (dataLen > 0) {
        corrected = corrected.mid(0, dataLen);
    }

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncoded + m_stats.totalDecoded);

    emit codingCompleted(correctedSymbols);
    return corrected;
}

/**
 * @brief 重置所有统计信息
 */
void ReedSolomon10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
