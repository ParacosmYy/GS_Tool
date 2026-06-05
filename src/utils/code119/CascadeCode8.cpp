#include "CascadeCode8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化级联码编解码引擎
 * @param parent 父对象指针
 */
CascadeCode8::CascadeCode8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void CascadeCode8::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 简化的外码编码（类似RS编码）
 *
 * 对数据块添加纠错校验位，使用GF(2)上的线性分组码。
 * 校验位为数据位的模2累加和的变体。
 *
 * @param data 输入数据比特
 * @param ecLen 纠错校验长度
 * @return 编码后序列（数据+校验）
 */
static QVector<int> outerEncode(const QVector<int>& data, int ecLen)
{
    QVector<int> result = data;
    for (int i = 0; i < ecLen; ++i) {
        int parity = 0;
        for (int j = i; j < data.size(); j += ecLen + 1) {
            parity ^= data[j];
        }
        result.append(parity);
    }
    return result;
}

/**
 * @brief 简化的外码解码
 *
 * 基于校验方程检测并纠正错误，对每个校验组独立修复。
 * 返回纠正后的数据（不含校验位）和纠错数量。
 *
 * @param received 接收序列（含校验）
 * @param ecLen 纠错校验长度
 * @param dataLen 原始数据长度
 * @param correctedErrors 纠正的错误数（输出参数）
 * @return 纠正后的数据比特
 */
static QVector<int> outerDecode(const QVector<double>& received, int ecLen,
                                 int dataLen, int& correctedErrors)
{
    correctedErrors = 0;
    QVector<int> hardBits;
    hardBits.reserve(received.size());
    for (int i = 0; i < received.size(); ++i) {
        hardBits.append((received[i] < 0.0) ? 1 : 0);
    }

    if (dataLen <= 0 || ecLen <= 0) return hardBits;

    QVector<int> data(hardBits.constBegin(), hardBits.constBegin() + qMin(dataLen, hardBits.size()));

    for (int i = 0; i < ecLen; ++i) {
        int parity = 0;
        for (int j = i; j < data.size(); j += ecLen + 1) {
            parity ^= data[j];
        }
        int checkIdx = dataLen + i;
        int expected = (checkIdx < hardBits.size()) ? hardBits[checkIdx] : 0;
        if (parity != expected) {
            for (int j = i; j < data.size(); j += ecLen + 1) {
                if (j < data.size()) {
                    data[j] ^= 1;
                    correctedErrors++;
                    break;
                }
            }
        }
    }
    return data;
}

/**
 * @brief 简化的卷积内码编码
 *
 * 基于约束长度和生成多项式进行卷积编码。
 * 使用移位寄存器实现多项式除法。
 *
 * @param data 输入比特
 * @param constraint 约束长度
 * @param genPolys 生成多项式列表
 * @return 编码后序列（每输入比特产生genPolys.size()个输出）
 */
static QVector<int> convEncode(const QVector<int>& data, int constraint,
                                const QVector<int>& genPolys)
{
    QVector<int> result;
    int state = 0;
    int mask = (1 << (constraint - 1)) - 1;

    for (int bit : data) {
        state = ((state << 1) | bit) & mask;
        for (int poly : genPolys) {
            int out = 0;
            int s = state | (bit << (constraint - 1));
            for (int k = 0; k < constraint; ++k) {
                if ((poly >> k) & 1) out ^= ((s >> k) & 1);
            }
            result.append(out);
        }
    }
    return result;
}

/**
 * @brief 简化的卷积内码解码（硬判决Viterbi）
 *
 * 基于欧氏距离的硬判决Viterbi解码，在网格图中选择
 * 最优路径以恢复原始数据。
 *
 * @param received 接收序列
 * @param constraint 约束长度
 * @param genPolys 生成多项式列表
 * @param dataLen 原始数据长度
 * @return 解码后的数据比特
 */
static QVector<int> convDecode(const QVector<double>& received, int constraint,
                                const QVector<int>& genPolys, int dataLen)
{
    int numStates = 1 << (constraint - 1);
    int rateInv = genPolys.size();
    int symLen = dataLen;
    if (symLen * rateInv > received.size()) {
        symLen = received.size() / qMax(1, rateInv);
    }

    QVector<double> pathMetric(numStates, 1e18);
    QVector<QVector<int>> pathHistory(numStates);
    pathMetric[0] = 0.0;

    for (int t = 0; t < symLen; ++t) {
        QVector<double> newMetric(numStates, 1e18);
        QVector<QVector<int>> newHistory(numStates);

        for (int s = 0; s < numStates; ++s) {
            if (pathMetric[s] > 1e17) continue;
            for (int inp = 0; inp <= 1; ++inp) {
                int ns = ((s << 1) | inp) & (numStates - 1);
                int sFull = (s << 1) | inp;
                double dist = 0.0;
                for (int g = 0; g < rateInv; ++g) {
                    int expected = 0;
                    for (int k = 0; k < constraint; ++k) {
                        if ((genPolys[g] >> k) & 1) expected ^= ((sFull >> k) & 1);
                    }
                    int idx = t * rateInv + g;
                    double rx = (idx < received.size()) ? received[idx] : 0.0;
                    double diff = rx - (expected * 2.0 - 1.0);
                    dist += diff * diff;
                }
                double metric = pathMetric[s] + dist;
                if (metric < newMetric[ns]) {
                    newMetric[ns] = metric;
                    newHistory[ns] = pathHistory[s];
                    newHistory[ns].append(inp);
                }
            }
        }
        pathMetric = newMetric;
        pathHistory = newHistory;
    }

    int bestState = 0;
    double bestMetric = 1e18;
    for (int s = 0; s < numStates; ++s) {
        if (pathMetric[s] < bestMetric) {
            bestMetric = pathMetric[s];
            bestState = s;
        }
    }

    QVector<int> result = pathHistory[bestState];
    if (result.size() > dataLen) result.resize(dataLen);
    return result;
}

/**
 * @brief 块交织器
 *
 * 将数据按行列写入，按列行读出，打散突发错误。
 *
 * @param data 输入数据
 * @param rows 行数
 * @param cols 列数
 * @return 交织后的数据
 */
static QVector<double> blockInterleave(const QVector<double>& data, int rows, int cols)
{
    int n = qMin(data.size(), rows * cols);
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        int row = i / cols;
        int col = i % cols;
        int outIdx = col * rows + row;
        if (outIdx < n && i < data.size()) {
            result[outIdx] = data[i];
        }
    }
    return result;
}

/**
 * @brief 块解交织器
 *
 * 交织的逆操作：按列行写入，按行列读出。
 *
 * @param data 输入数据
 * @param rows 行数
 * @param cols 列数
 * @return 解交织后的数据
 */
static QVector<double> blockDeinterleave(const QVector<double>& data, int rows, int cols)
{
    int n = qMin(data.size(), rows * cols);
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        int col = i / rows;
        int row = i % rows;
        int outIdx = row * cols + col;
        if (outIdx < n && i < data.size()) {
            result[outIdx] = data[i];
        }
    }
    return result;
}

/**
 * @brief 级联编码（先外码后内码）
 *
 * 流程：原始数据 → 外码编码 → 块交织 → 内码编码。
 * 外码负责纠正分散错误，内码负责纠正随机错误。
 *
 * @param dataBits 原始信息比特
 * @param outerCode 外码类型 (RS/BCH)
 * @param innerCode 内码类型 (Conv/LDPC)
 * @return 级联编码后的码字
 */
QVector<int> CascadeCode8::encode(const QVector<int>& dataBits,
                                   const QString& outerCode, const QString& innerCode)
{
    QElapsedTimer timer;
    timer.start();

    if (dataBits.isEmpty()) {
        emit decodeCompleted(0);
        return {};
    }

    Q_UNUSED(outerCode)
    Q_UNUSED(innerCode)

    /* 外码编码：添加2t个校验位（默认t=4） */
    int ecLen = 8;
    QVector<int> outerEncoded = outerEncode(dataBits, ecLen);

    /* 块交织 */
    int rows = 8;
    int cols = (outerEncoded.size() + rows - 1) / rows;
    QVector<double> interleaved(cols * rows, 0.0);
    for (int i = 0; i < outerEncoded.size() && i < interleaved.size(); ++i) {
        interleaved[i] = outerEncoded[i] * 2.0 - 1.0;
    }
    interleaved = blockInterleave(interleaved, rows, cols);

    /* 内码编码：卷积码 */
    QVector<int> innerInput;
    for (double v : interleaved) {
        innerInput.append((v >= 0.0) ? 0 : 1);
    }
    QVector<int> genPolys = {07, 05};
    QVector<int> result = convEncode(innerInput, 3, genPolys);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(result.size());
    return result;
}

/**
 * @brief 级联迭代解码
 *
 * 流程：内码解码 → 解交织 → 外码解码。
 * 支持多次外层迭代以提升纠错性能。
 *
 * @param received 接收序列
 * @param maxOuterIterations 外层迭代次数
 * @return 解码后的信息比特
 */
QVector<int> CascadeCode8::decode(const QVector<double>& received, int maxOuterIterations)
{
    QElapsedTimer timer;
    timer.start();

    if (received.isEmpty()) {
        emit decodeCompleted(0);
        return {};
    }

    maxOuterIterations = qMax(1, maxOuterIterations);
    int ecLen = 8;
    int constraint = 3;
    QVector<int> genPolys = {07, 05};
    int rateInv = genPolys.size();

    int innerDataLen = received.size() / qMax(1, rateInv);
    int rows = 8;
    int cols = (innerDataLen + rows - 1) / rows;
    int outerEncodedLen = rows * cols;

    QVector<double> currentReceived = received;
    int totalCorrected = 0;

    for (int iter = 0; iter < maxOuterIterations; ++iter) {
        /* 内码解码 */
        QVector<int> innerDecoded = convDecode(currentReceived, constraint, genPolys, innerDataLen);

        /* 转为软信息 */
        QVector<double> softOutput(innerDecoded.size());
        for (int i = 0; i < innerDecoded.size(); ++i) {
            softOutput[i] = innerDecoded[i] * 2.0 - 1.0;
        }

        /* 填充到交织长度 */
        QVector<double> deinterleaved(outerEncodedLen, 0.0);
        for (int i = 0; i < qMin(softOutput.size(), outerEncodedLen); ++i) {
            deinterleaved[i] = softOutput[i];
        }
        deinterleaved = blockDeinterleave(deinterleaved, rows, cols);

        /* 外码解码 */
        int dataLen = deinterleaved.size() - ecLen;
        int corrected = 0;
        QVector<int> outerDecoded = outerDecode(deinterleaved, ecLen, dataLen, corrected);
        totalCorrected += corrected;

        if (iter < maxOuterIterations - 1) {
            /* 重编码用于下一次迭代 */
            QVector<int> reEncoded = outerEncode(outerDecoded, ecLen);
            QVector<double> reInterleaved(rows * cols, 0.0);
            for (int i = 0; i < reEncoded.size() && i < reInterleaved.size(); ++i) {
                reInterleaved[i] = reEncoded[i] * 2.0 - 1.0;
            }
            reInterleaved = blockInterleave(reInterleaved, rows, cols);
            QVector<double> reInner(received.size(), 0.0);
            QVector<int> reInnerBits;
            for (double v : reInterleaved) {
                reInnerBits.append((v >= 0.0) ? 0 : 1);
            }
            QVector<int> reEncodedInner = convEncode(reInnerBits, constraint, genPolys);
            for (int i = 0; i < qMin(reEncodedInner.size(), (int)received.size()); ++i) {
                double prior = (i < received.size()) ? received[i] : 0.0;
                double recon = reEncodedInner[i] * 2.0 - 1.0;
                currentReceived[i] = prior * 0.7 + recon * 0.3;
            }
        } else {
            qint64 elapsed = timer.elapsed();
            m_timeSum += elapsed;
            m_stats.totalDecodeOps++;
            m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

            emit decodeCompleted(totalCorrected);
            return outerDecoded;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(totalCorrected);
    return {};
}

/**
 * @brief 设置外码参数
 *
 * @param codeLength 码长
 * @param errorCorrection 纠错能力（可纠正符号数）
 */
void CascadeCode8::setOuterCodeParams(int codeLength, int errorCorrection)
{
    Q_UNUSED(codeLength)
    Q_UNUSED(errorCorrection)
    /* 参数预留接口，当前使用默认值 */
}

/**
 * @brief 设置内码参数
 *
 * @param constraintLength 约束长度
 * @param generatorPolynomials 生成多项式（八进制表示）
 */
void CascadeCode8::setInnerCodeParams(int constraintLength,
                                       const QVector<int>& generatorPolynomials)
{
    Q_UNUSED(constraintLength)
    Q_UNUSED(generatorPolynomials)
    /* 参数预留接口，当前使用默认值 */
}
