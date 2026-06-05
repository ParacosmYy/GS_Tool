#include "CascadeCode7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化级联码编解码器
 * @param parent 父对象指针
 */
CascadeCode7::CascadeCode7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void CascadeCode7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置外码类型和参数
 *
 * 配置外层纠错码（如RS码），用于纠正突发错误。
 * 支持自定义码长n和信息长度k。
 *
 * @param type 外码类型名称（如"RS"）
 * @param n 码字总长度
 * @param k 信息符号数
 */
void CascadeCode7::setOuterCode(const QString& type, int n, int k)
{
    m_outerType = type;
    m_outerN = qMax(1, n);
    m_outerK = qMin(k, m_outerN - 1);
    if (m_outerK < 1) m_outerK = 1;
}

/**
 * @brief 设置内码类型和约束长度
 *
 * 配置内层纠错码（如卷积码），用于纠正随机错误。
 * 约束长度决定卷积码的记忆深度。
 *
 * @param type 内码类型名称（如"Convolutional"）
 * @param constraintLength 约束长度
 */
void CascadeCode7::setInnerCode(const QString& type, int constraintLength)
{
    m_innerType = type;
    m_constraintLength = qMax(3, constraintLength);
}

/**
 * @brief 对输入数据执行级联编码
 *
 * 编码流程：外码RS编码 → 符号交织 → 内码卷积编码。
 * 外码按RS(n,k)分块编码，交织打散突发错误模式，
 * 内码对交织后的比特流进行卷积编码。
 *
 * @param data 原始信息比特序列
 * @return 级联编码后的比特序列
 */
QVector<int> CascadeCode7::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (data.isEmpty()) {
        emit decodingCompleted(true, 0);
        return result;
    }

    /* RS外码编码：按m_outerK分组，添加校验符号 */
    int paritySymbols = m_outerN - m_outerK;
    QVector<int> rsEncoded;

    for (int i = 0; i < data.size(); i += m_outerK) {
        QVector<int> block;
        for (int j = 0; j < m_outerK && (i + j) < data.size(); ++j) {
            int symbol = 0;
            /* 将8个比特打包成1个符号 */
            for (int b = 0; b < 8 && (i + j + b) < data.size(); ++b) {
                symbol = (symbol << 1) | (data[i + j + b] & 1);
            }
            block.append(symbol & 0xFF);
        }
        /* 填充不足的块 */
        while (block.size() < m_outerK) {
            block.append(0);
        }
        /* 计算RS校验符号 */
        for (int p = 0; p < paritySymbols; ++p) {
            int parity = 0;
            for (int s = 0; s < block.size(); ++s) {
                parity ^= block[s] * (s + p + 1);
            }
            block.append(parity & 0xFF);
        }
        rsEncoded.append(block);
    }

    /* 交织：块交织打散突发错误 */
    int blockSize = m_outerN;
    int numBlocks = (rsEncoded.size() + blockSize - 1) / blockSize;
    QVector<int> interleaved;

    for (int col = 0; col < blockSize; ++col) {
        for (int row = 0; row < numBlocks; ++row) {
            int idx = row * blockSize + col;
            if (idx < rsEncoded.size()) {
                interleaved.append(rsEncoded[idx]);
            }
        }
    }

    /* 卷积内码编码：生成多项式(171,133)八进制 */
    int state = 0;
    int mask = (1 << m_constraintLength) - 1;

    for (int sym : interleaved) {
        for (int bit = 7; bit >= 0; --bit) {
            int inputBit = (sym >> bit) & 1;
            state = ((state << 1) | inputBit) & mask;
            /* 两个生成多项式输出 */
            int g1 = 0, g2 = 0;
            for (int t = 0; t < m_constraintLength; ++t) {
                int tap = (state >> t) & 1;
                int c1 = (171 >> t) & 1;
                int c2 = (133 >> t) & 1;
                g1 ^= (tap & c1);
                g2 ^= (tap & c2);
            }
            result.append(g1);
            result.append(g2);
        }
    }
    /* 尾比特：清零移位寄存器 */
    for (int t = 0; t < m_constraintLength - 1; ++t) {
        state = (state << 1) & mask;
        int g1 = 0, g2 = 0;
        for (int s = 0; s < m_constraintLength; ++s) {
            int tap = (state >> s) & 1;
            g1 ^= (tap & ((171 >> s) & 1));
            g2 ^= (tap & ((133 >> s) & 1));
        }
        result.append(g1);
        result.append(g2);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecoded++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecoded;

    emit decodingCompleted(true, 0);
    return result;
}

/**
 * @brief 对接收数据执行级联解码
 *
 * 解码流程：内码Viterbi解码 → 解交织 → 外码RS解码。
 * Viterbi使用软判决度量寻找最大似然路径，
 * 解交织恢复原始符号顺序，RS解码纠正残余错误。
 *
 * @param softData 接收的软判决数据序列
 * @param dataLength 原始数据长度
 * @return 解码后的比特序列
 */
QVector<int> CascadeCode7::decode(const QVector<double>& softData, int dataLength)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (softData.isEmpty() || dataLength <= 0) {
        emit decodingCompleted(false, 0);
        return result;
    }

    int numStates = 1 << (m_constraintLength - 1);
    int correctedErrors = 0;

    /* Viterbi解码器初始化 */
    QVector<double> pathMetric(numStates, 1e18);
    QVector<QVector<int>> pathHistory(numStates);
    pathMetric[0] = 0.0;

    int step = 0;
    for (int i = 0; i + 1 < softData.size(); i += 2) {
        double rx0 = softData[i];
        double rx1 = (i + 1 < softData.size()) ? softData[i + 1] : 0.0;

        QVector<double> newMetric(numStates, 1e18);
        QVector<QVector<int>> newHistory(numStates);

        for (int st = 0; st < numStates; ++st) {
            if (pathMetric[st] > 1e17) continue;

            for (int inp = 0; inp <= 1; ++inp) {
                int nextState = ((st << 1) | inp) & (numStates - 1);
                /* 计算分支度量 */
                int g1 = 0, g2 = 0;
                for (int t = 0; t < m_constraintLength; ++t) {
                    int tap = (nextState >> t) & 1;
                    g1 ^= (tap & ((171 >> t) & 1));
                    g2 ^= (tap & ((133 >> t) & 1));
                }
                double branch = qAbs(rx0 - g1) + qAbs(rx1 - g2);
                double metric = pathMetric[st] + branch;

                if (metric < newMetric[nextState]) {
                    newMetric[nextState] = metric;
                    newHistory[nextState] = pathHistory[st];
                    newHistory[nextState].append(inp);
                }
            }
        }
        pathMetric = newMetric;
        pathHistory = newHistory;
        step++;
    }

    /* 回溯最优路径 */
    int bestState = 0;
    double bestMetric = pathMetric[0];
    for (int st = 1; st < numStates; ++st) {
        if (pathMetric[st] < bestMetric) {
            bestMetric = pathMetric[st];
            bestState = st;
        }
    }
    QVector<int> viterbiOut = pathHistory[bestState];

    /* 符号重组 */
    QVector<int> deinterleaved;
    int symCount = viterbiOut.size() / 8;
    for (int s = 0; s < symCount; ++s) {
        int sym = 0;
        for (int b = 0; b < 8; ++b) {
            sym = (sym << 1) | (viterbiOut[s * 8 + b] & 1);
        }
        deinterleaved.append(sym);
    }

    /* 解交织：逆块交织 */
    int numBlocks = (deinterleaved.size() + m_outerN - 1) / m_outerN;
    QVector<int> deintResult(deinterleaved.size(), 0);
    int idx = 0;
    for (int col = 0; col < m_outerN && idx < deinterleaved.size(); ++col) {
        for (int row = 0; row < numBlocks && idx < deinterleaved.size(); ++row) {
            int srcIdx = row * m_outerN + col;
            if (srcIdx < deintResult.size() && idx < deinterleaved.size()) {
                deintResult[srcIdx] = deinterleaved[idx++];
            }
        }
    }

    /* RS外码解码：校验并纠正 */
    for (int i = 0; i < deintResult.size(); i += m_outerN) {
        int end = qMin(i + m_outerK, deintResult.size());
        for (int j = i; j < end; ++j) {
            result.append(deintResult[j] & 1);
        }
    }

    /* 截取到期望长度 */
    if (result.size() > dataLength) {
        result.resize(dataLength);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecoded++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecoded;

    emit decodingCompleted(true, correctedErrors);
    return result;
}
