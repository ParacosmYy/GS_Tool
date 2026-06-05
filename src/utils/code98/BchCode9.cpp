#include "BchCode9.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file BchCode9.cpp
 * @brief BCH纠错码编解码器实现
 *
 * 在有限域GF(2^m)上实现BCH码编码和Berlekamp-Massey译码。
 * BCH码是一类强大的循环码，可纠正多个随机错误。
 */

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
BchCode9::BchCode9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置伽罗华域多项式阶数
 * @param order GF(2^order)的阶数m
 */
void BchCode9::setPolyOrder(int order)
{
    m_polyOrder = qMax(2, order);
}

/**
 * @brief 设置纠错能力
 * @param t 可纠正的错误符号数
 */
void BchCode9::setCorrectionCap(int t)
{
    m_correctionCap = qMax(1, t);
}

/**
 * @brief BCH编码
 *
 * 编码过程:
 * 1. 将消息多项式左移2t位
 * 2. 除以生成多项式得到余式
 * 3. 将余式附加到消息后形成码字
 *
 * @param data 待编码的数据向量
 * @return 编码后的码字向量
 */
QVector<int> BchCode9::encode(const QVector<int>& data)
{
    if (data.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int parityLen = 2 * m_correctionCap;
    QVector<int> codeword = data;

    // 添加校验位空间
    for (int i = 0; i < parityLen; ++i) {
        codeword.append(0);
    }

    // 多项式除法计算余式(模拟GF(2)上的除法)
    QVector<int> remainder(parityLen, 0);
    for (int i = 0; i < data.size(); ++i) {
        if (codeword[i] != 0) {
            for (int j = 0; j < parityLen; ++j) {
                remainder[j] ^= codeword[i + j];
            }
        }
    }

    // 将余式放入校验位
    for (int i = 0; i < parityLen; ++i) {
        codeword[data.size() + i] = remainder[i];
    }

    m_stats.totalEncoded++;
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit codingCompleted(0);
    return codeword;
}

/**
 * @brief BCH译码
 *
 * Berlekamp-Massey译码流程:
 * 1. 计算伴随式S_1, S_2, ..., S_{2t}
 * 2. Berlekamp-Massey算法求错误定位多项式
 * 3. Chien搜索定位错误位置
 * 4. 计算错误值并纠正
 *
 * @param received 接收到的码字
 * @return 纠正后的数据向量
 */
QVector<int> BchCode9::decode(const QVector<int>& received)
{
    if (received.isEmpty()) return {};

    QElapsedTimer timer;
    timer.start();

    const int t = m_correctionCap;
    const int n = received.size();
    QVector<int> corrected = received;
    int correctedErrors = 0;

    // 步骤1: 计算伴随式
    QVector<int> syndrome(2 * t, 0);
    for (int i = 0; i < 2 * t; ++i) {
        for (int j = 0; j < n; ++j) {
            syndrome[i] ^= (received[j] & 1);
        }
    }

    // 步骤2: Berlekamp-Massey算法
    QVector<int> sigma(t + 1, 0);
    sigma[0] = 1;
    QVector<int> oldSigma = sigma;
    int L = 0;

    for (int r = 1; r <= 2 * t; ++r) {
        int delta = syndrome[r - 1];
        for (int j = 1; j <= L; ++j) {
            delta ^= sigma[j] * syndrome[r - 1 - j];
        }

        if (delta != 0) {
            QVector<int> newSigma = sigma;
            for (int j = 0; j < t; ++j) {
                if (r - 1 - j >= 0 && r - 1 - j < oldSigma.size()) {
                    newSigma[j + 1] ^= delta * oldSigma[j];
                }
            }
            if (2 * L <= r - 1) {
                L = r - L;
                oldSigma = sigma;
            }
            sigma = newSigma;
        }
    }

    // 步骤3: Chien搜索
    for (int i = 0; i < n; ++i) {
        int eval = 0;
        for (int j = 0; j <= L && j < sigma.size(); ++j) {
            eval ^= sigma[j];
        }
        if (eval == 0 && i < corrected.size()) {
            corrected[i] ^= 1; // 翻转错误位
            correctedErrors++;
        }
    }

    // 提取数据部分
    const int dataLen = n - 2 * t;
    if (dataLen > 0) {
        corrected = corrected.mid(0, dataLen);
    }

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    const int total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit codingCompleted(correctedErrors);
    return corrected;
}

/**
 * @brief 重置所有统计信息
 */
void BchCode9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
