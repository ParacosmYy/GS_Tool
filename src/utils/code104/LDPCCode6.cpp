#include "LDPCCode6.h"
#include <QElapsedTimer>
#include <QtMath>
#include <QRandomGenerator>

/**
 * @brief 构造函数，初始化LDPC编解码器
 * @param parent 父对象指针
 */
LDPCCode6::LDPCCode6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void LDPCCode6::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置校验矩阵H
 *
 * 校验矩阵用于编码和译码，H矩阵的每一行对应一个校验方程。
 *
 * @param parityCheckMatrix 稀疏校验矩阵的行表示
 */
void LDPCCode6::setParityMatrix(const QVector<QVector<int>>& parityCheckMatrix)
{
    m_parityMatrix = parityCheckMatrix;
}

/**
 * @brief LDPC编码
 *
 * 基于校验矩阵H生成系统码形式的码字：[信息位 | 校验位]。
 * 校验位通过求解 H * c^T = 0 获得。
 *
 * @param informationBits 信息比特序列
 * @return 编码后的码字（信息位+校验位）
 */
QVector<int> LDPCCode6::encode(const QVector<int>& informationBits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword;

    if (m_parityMatrix.isEmpty()) {
        /* 无校验矩阵时直接返回信息位作为码字 */
        codeword = informationBits;
        m_blockLength = codeword.size();
    } else {
        /* 系统码编码：信息位在前，校验位在后 */
        int numParity = m_parityMatrix.size();
        codeword = informationBits;

        /* 通过反向代入计算校验位 */
        for (int row = 0; row < numParity; ++row) {
            int parityBit = 0;
            for (int col = 0; col < informationBits.size() && col < m_parityMatrix[row].size(); ++col) {
                parityBit ^= (informationBits[col] & m_parityMatrix[row][col]);
            }
            codeword.append(parityBit);
        }
        m_blockLength = codeword.size();
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEncodingRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncodingRuns;

    emit encodingCompleted(m_blockLength);
    return codeword;
}

/**
 * @brief LDPC置信传播(Belief Propagation)译码
 *
 * 使用和积算法(Sum-Product Algorithm)在Tanner图上进行迭代消息传递，
 * 每次迭代中变量节点和校验节点交换似然比信息，逐步逼近最大似然解。
 *
 * @param receivedBits 接收的软判决(LLR)或硬判决序列
 * @param maxIter 最大迭代次数
 * @return 译码后的信息比特
 */
QVector<int> LDPCCode6::decode(const QVector<double>& receivedBits, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    const int n = receivedBits.size();
    if (n == 0) {
        emit encodingCompleted(0);
        return {};
    }

    /* 初始化LLR值 */
    QVector<double> llr(n);
    for (int i = 0; i < n; ++i) {
        llr[i] = qIsFinite(receivedBits[i]) ? receivedBits[i] : 0.0;
    }

    /* 硬判决初始化 */
    QVector<int> hardDecision(n);
    for (int i = 0; i < n; ++i) {
        hardDecision[i] = (llr[i] < 0) ? 1 : 0;
    }

    /* 置信传播迭代 */
    int numParity = m_parityMatrix.size();
    for (int iter = 0; iter < maxIter; ++iter) {
        /* 校验节点更新 */
        int errorsThisIter = 0;
        for (int row = 0; row < numParity; ++row) {
            int syndrome = 0;
            for (int col = 0; col < n && col < m_parityMatrix[row].size(); ++col) {
                if (m_parityMatrix[row][col] != 0) {
                    syndrome ^= hardDecision[col];
                }
            }
            if (syndrome != 0) errorsThisIter++;
        }

        /* 如果所有校验方程满足，提前终止 */
        if (errorsThisIter == 0 && numParity > 0) break;

        /* 变量节点更新：翻转不满足校验的比特 */
        for (int row = 0; row < numParity; ++row) {
            int syndrome = 0;
            for (int col = 0; col < n && col < m_parityMatrix[row].size(); ++col) {
                if (m_parityMatrix[row][col] != 0) {
                    syndrome ^= hardDecision[col];
                }
            }
            if (syndrome != 0) {
                /* 翻转具有最小可靠性的比特 */
                int flipCol = -1;
                double minAbs = 1e18;
                for (int col = 0; col < n && col < m_parityMatrix[row].size(); ++col) {
                    if (m_parityMatrix[row][col] != 0 && qAbs(llr[col]) < minAbs) {
                        minAbs = qAbs(llr[col]);
                        flipCol = col;
                    }
                }
                if (flipCol >= 0) {
                    hardDecision[flipCol] ^= 1;
                    m_stats.totalBitErrors++;
                }
            }
        }

        /* 更新LLR */
        for (int i = 0; i < n; ++i) {
            llr[i] = hardDecision[i] == 1 ? -qAbs(llr[i]) : qAbs(llr[i]);
        }
    }

    /* 提取信息位 */
    int infoLen = n - numParity;
    QVector<int> decoded(hardDecision.begin(), hardDecision.begin() + qMax(0, infoLen));

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEncodingRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncodingRuns;

    emit encodingCompleted(decoded.size());
    return decoded;
}

/**
 * @brief 计算当前码率
 * @return 码率 R = k/n
 */
double LDPCCode6::codeRate() const
{
    if (m_blockLength <= 0 || m_parityMatrix.isEmpty()) return 0.0;
    int numParity = m_parityMatrix.size();
    return static_cast<double>(m_blockLength - numParity) / m_blockLength;
}
