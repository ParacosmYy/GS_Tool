#include "PolarCode13.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化极化码编解码引擎
 * @param parent 父对象指针
 */
PolarCode13::PolarCode13(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void PolarCode13::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief Polar码编码
 *
 * 基于递归蝴蝶结构的XOR极化变换。
 * 将信息位放置在选定的信道位置，冻结位填充0，
 * 然后通过逐级XOR生成码字。
 *
 * @param infoBits 信息比特序列
 * @param codeLength 码长（2的幂）
 * @return 编码后的码字
 */
QVector<int> PolarCode13::encode(const QVector<int>& infoBits, int codeLength)
{
    QElapsedTimer timer;
    timer.start();

    if (infoBits.isEmpty() || codeLength <= 0) {
        emit decodeCompleted(0);
        return {};
    }

    /* 确保码长为2的幂 */
    int n = 1;
    while (n < codeLength) n *= 2;
    codeLength = n;

    /* 构造u向量：信息位放在前infoBits.size()位，其余填冻结位0 */
    QVector<int> u(codeLength, 0);
    for (int i = 0; i < qMin(infoBits.size(), codeLength); ++i) {
        u[i] = (infoBits[i] != 0) ? 1 : 0;
    }

    /* 递归蝴蝶变换 */
    QVector<int> coded = u;
    for (int stage = 1; stage < codeLength; stage *= 2) {
        for (int i = 0; i < codeLength; i += 2 * stage) {
            for (int j = 0; j < stage; ++j) {
                if (i + j < codeLength && i + j + stage < codeLength) {
                    int a = coded[i + j];
                    int b = coded[i + j + stage];
                    coded[i + j] = a ^ b;
                    coded[i + j + stage] = b;
                }
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(codeLength);
    return coded;
}

/**
 * @brief SC（连续消除）解码
 *
 * 逐比特顺序解码，利用LLR树递归计算。
 * 每个比特根据LLR值的符号进行硬判决，
 * 冻结位直接判为0。
 *
 * @param llrValues 对数似然比序列
 * @return 解码后的信息比特
 */
QVector<int> PolarCode13::decodeSC(const QVector<double>& llrValues)
{
    QElapsedTimer timer;
    timer.start();

    if (llrValues.isEmpty()) {
        emit decodeCompleted(0);
        return {};
    }

    int n = llrValues.size();
    QVector<int> decoded(n, 0);

    /* 简化的SC解码：逐层计算部分和 */
    QVector<double> llr = llrValues;
    int halfN = n / 2;

    /* 前向LLR传播 */
    for (int stage = halfN; stage >= 1; stage /= 2) {
        QVector<double> newLlr(n, 0.0);
        for (int i = 0; i < n; i += 2 * stage) {
            for (int j = 0; j < stage; ++j) {
                if (i + j + stage < n && (i + j + stage) < llr.size()) {
                    double la = llr[i + j];
                    double lb = llr[i + j + stage];
                    double signProd = ((la >= 0) ? 1.0 : -1.0) * ((lb >= 0) ? 1.0 : -1.0);
                    double minAbs = qMin(qAbs(la), qAbs(lb));
                    newLlr[i + j] = signProd * minAbs;
                    newLlr[i + j + stage] = lb;
                }
            }
        }
        llr = newLlr;
    }

    /* 硬判决 */
    for (int i = 0; i < n; ++i) {
        decoded[i] = (llr[i] < 0) ? 1 : 0;
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(1);
    return decoded;
}

/**
 * @brief SCL（连续消除列表）解码
 *
 * 维护最多listSize条候选路径，每步复制并扩展。
 * 路径度量基于LLR的绝对值累加，保留最优的listSize条路径。
 *
 * @param llrValues 对数似然比序列
 * @param listSize 列表大小
 * @return 解码后的信息比特
 */
QVector<int> PolarCode13::decodeSCL(const QVector<double>& llrValues, int listSize)
{
    QElapsedTimer timer;
    timer.start();

    if (llrValues.isEmpty()) {
        emit decodeCompleted(0);
        return {};
    }

    int n = llrValues.size();
    listSize = qMax(1, listSize);

    /* 路径结构：每个路径保存已解码比特和度量值 */
    struct Path {
        QVector<int> bits;
        double metric;
    };

    QVector<Path> paths;
    paths.append({QVector<int>(n, 0), 0.0});

    /* 逐比特解码 */
    for (int bitIdx = 0; bitIdx < n; ++bitIdx) {
        QVector<Path> newPaths;

        for (const auto& path : paths) {
            double llr = (bitIdx < llrValues.size()) ? llrValues[bitIdx] : 0.0;
            /* 候选0和候选1 */
            for (int bitVal = 0; bitVal <= 1; ++bitVal) {
                Path np = path;
                np.bits[bitIdx] = bitVal;
                /* 度量更新：LLR与硬判决不一致时增加惩罚 */
                double penalty = (bitVal == 0) ? qMax(0.0, -llr) : qMax(0.0, llr);
                np.metric += penalty;
                newPaths.append(np);
            }
        }

        /* 按度量排序，保留前listSize条 */
        std::sort(newPaths.begin(), newPaths.end(),
                  [](const Path& a, const Path& b) { return a.metric < b.metric; });
        if (newPaths.size() > listSize) {
            newPaths.resize(listSize);
        }
        paths = newPaths;
    }

    QVector<int> result = paths.isEmpty() ? QVector<int>(n, 0) : paths.first().bits;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(listSize);
    return result;
}

/**
 * @brief 基于Bhattacharyya参数选择信息位位置
 *
 * 递归计算每个子信道的Bhattacharyya参数，
 * 参数值越小表示信道越可靠，选为信息位。
 * 排序后取最小的infoLength个作为信息位索引。
 *
 * @param codeLength 码长
 * @param infoLength 信息位长度
 * @param designSnrDb 设计信噪比 (dB)
 * @return 信息位索引集合（升序排列）
 */
QVector<int> PolarCode13::selectInfoBits(int codeLength, int infoLength, double designSnrDb)
{
    QElapsedTimer timer;
    timer.start();

    if (codeLength <= 0 || infoLength <= 0 || infoLength > codeLength) {
        emit decodeCompleted(0);
        return {};
    }

    /* 确保码长为2的幂 */
    int n = 1;
    while (n < codeLength) n *= 2;
    codeLength = n;

    /* 初始Bhattacharyya参数从设计SNR计算 */
    double snrLinear = qPow(10.0, designSnrDb / 10.0);
    double zInit = qExp(-snrLinear);

    QVector<double> z(1, zInit);

    /* 递归计算：z' = 2z - z^2, z'' = z^2 */
    while (z.size() < codeLength) {
        QVector<double> newZ;
        newZ.reserve(z.size() * 2);
        for (int i = 0; i < z.size(); ++i) {
            double zi = z[i];
            newZ.append(2.0 * zi - zi * zi);
            newZ.append(zi * zi);
        }
        z = newZ;
    }

    /* 创建索引并按Bhattacharyya参数排序 */
    QVector<QPair<double, int>> indexed;
    indexed.reserve(z.size());
    for (int i = 0; i < z.size(); ++i) {
        indexed.append({z[i], i});
    }
    std::sort(indexed.begin(), indexed.end(),
              [](const QPair<double, int>& a, const QPair<double, int>& b) {
                  return a.first < b.first;
              });

    /* 取前infoLength个索引 */
    QVector<int> infoIndices;
    infoIndices.reserve(infoLength);
    for (int i = 0; i < qMin(infoLength, indexed.size()); ++i) {
        infoIndices.append(indexed[i].second);
    }
    std::sort(infoIndices.begin(), infoIndices.end());

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(infoIndices.size());
    return infoIndices;
}
