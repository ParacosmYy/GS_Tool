/**
 * @file WaveletPacket.cpp
 * @brief 小波包分解与重构实现 — Haar/DB2/DB4
 */

#include "utils/dwt4/WaveletPacket.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
WaveletPacket::WaveletPacket(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置小波基类型 @param type 小波基 */
void WaveletPacket::setWaveletType(WaveletType type)
{
    m_waveletType = type;
}

/** @brief 设置分解层数 @param levels 层数(1-10) */
void WaveletPacket::setDecompositionLevels(int levels)
{
    m_levels = qBound(1, levels, 10);
}

/** @brief 执行小波包分解
 *  @param signal 输入信号(长度应为2^N)
 *  @return 分解结果 */
WaveletPacket::DecompositionResult WaveletPacket::decompose(
    const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    DecompositionResult result;
    result.originalSize = signal.size();
    result.levels = m_levels;
    result.wavelet = m_waveletType;

    if (signal.isEmpty()) return result;

    /* 用队列实现完全二叉树分解 */
    struct WorkItem {
        QVector<double> data;
        int level;
        int index;
    };

    QList<WorkItem> queue;
    queue.append({signal, 0, 0});

    while (!queue.isEmpty()) {
        WorkItem item = queue.takeFirst();

        PacketNode node;
        node.level = item.level;
        node.index = item.index;
        node.coefficients = item.data;
        node.energy = computeEnergy(item.data);
        result.nodes.append(node);

        /* 继续分解直到目标层数 */
        if (item.level < m_levels && item.data.size() >= 2) {
            auto pair = singleLevelDecompose(item.data);
            queue.append({pair.first, item.level + 1, item.index * 2});
            queue.append({pair.second, item.level + 1, item.index * 2 + 1});
        }
    }

    ++m_stats.totalDecompositions;
    m_stats.totalSamplesProcessed += static_cast<quint64>(signal.size());

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecompositions);

    emit decompositionCompleted(m_levels, result.nodes.size());
    return result;
}

/** @brief 从分解结果重构信号
 *  @param result 分解结果
 *  @return 重构后的信号 */
QVector<double> WaveletPacket::reconstruct(
    const DecompositionResult& result)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> reconstructed;

    if (result.nodes.isEmpty()) return reconstructed;

    /* 从最高层的近似和细节系数重构 */
    /* 找到最后一层的节点对 */
    int lastLevel = result.levels;
    QList<const PacketNode*> lastNodes;
    for (const auto& node : result.nodes) {
        if (node.level == lastLevel) {
            lastNodes.append(&node);
        }
    }

    if (lastNodes.size() >= 2) {
        /* 按索引排序 */
        std::sort(lastNodes.begin(), lastNodes.end(),
                  [](const PacketNode* a, const PacketNode* b) {
                      return a->index < b->index;
                  });

        /* 逐对重构 */
        reconstructed = lastNodes[0]->coefficients;
        for (int i = 1; i < lastNodes.size() && i < lastNodes.size(); i += 2) {
            QVector<double> detail;
            if (i < lastNodes.size()) {
                detail = lastNodes[i]->coefficients;
            }
            reconstructed = singleLevelReconstruct(reconstructed, detail);
        }
    } else {
        /* 只有一层，直接用第一组近似+细节 */
        auto pair = singleLevelDecompose(
            result.nodes.first().coefficients);
        reconstructed = singleLevelReconstruct(pair.first, pair.second);
    }

    /* 截断到原始长度 */
    if (reconstructed.size() > result.originalSize) {
        reconstructed.resize(result.originalSize);
    }

    ++m_stats.totalReconstructions;

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);

    double error = 0.0;
    emit reconstructionCompleted(reconstructed.size(), error);
    return reconstructed;
}

/** @brief 单层小波分解
 *  @param signal 输入信号
 *  @return (近似系数, 细节系数) */
QPair<QVector<double>, QVector<double>>
WaveletPacket::singleLevelDecompose(const QVector<double>& signal)
{
    QVector<double> lowDec, highDec, lowRec, highRec;
    getFilters(m_waveletType, lowDec, highDec, lowRec, highRec);

    int n = signal.size();
    int half = n / 2;
    QVector<double> approx(half);
    QVector<double> detail(half);

    for (int i = 0; i < half; ++i) {
        double sumA = 0.0;
        double sumD = 0.0;
        for (int j = 0; j < lowDec.size(); ++j) {
            int idx = (2 * i + j) % n;
            sumA += signal[idx] * lowDec[j];
            sumD += signal[idx] * highDec[j];
        }
        approx[i] = sumA;
        detail[i] = sumD;
    }

    return {approx, detail};
}

/** @brief 单层小波重构
 *  @param approx 近似系数
 *  @param detail 细节系数
 *  @return 重构信号 */
QVector<double> WaveletPacket::singleLevelReconstruct(
    const QVector<double>& approx,
    const QVector<double>& detail)
{
    QVector<double> lowDec, highDec, lowRec, highRec;
    getFilters(m_waveletType, lowDec, highDec, lowRec, highRec);

    int half = qMin(approx.size(), detail.size());
    int n = half * 2;
    QVector<double> result(n, 0.0);

    for (int i = 0; i < half; ++i) {
        for (int j = 0; j < lowRec.size(); ++j) {
            int idx = (2 * i + j) % n;
            result[idx] += approx[i] * lowRec[j];
            result[idx] += detail[i] * highRec[j];
        }
    }

    return result;
}

/** @brief 计算各节点能量分布
 *  @param result 分解结果
 *  @return 节点能量向量 */
QVector<double> WaveletPacket::energySpectrum(
    const DecompositionResult& result) const
{
    QVector<double> energies;
    energies.reserve(result.nodes.size());
    for (const auto& node : result.nodes) {
        energies.append(node.energy);
    }
    return energies;
}

/** @brief 阈值去噪
 *  @param signal 输入信号
 *  @param threshold 阈值
 *  @return 去噪后信号 */
QVector<double> WaveletPacket::denoise(const QVector<double>& signal,
                                        double threshold)
{
    DecompositionResult decomp = decompose(signal);

    /* 对非最高层系数应用硬阈值 */
    for (auto& node : decomp.nodes) {
        if (node.level > 0) {
            for (int i = 0; i < node.coefficients.size(); ++i) {
                if (qAbs(node.coefficients[i]) < threshold) {
                    node.coefficients[i] = 0.0;
                }
            }
            node.energy = computeEnergy(node.coefficients);
        }
    }

    return reconstruct(decomp);
}

/** @brief 获取统计信息 */
WaveletPacket::Stats WaveletPacket::stats() const
{
    return m_stats;
}

/** @brief 重置统计 */
void WaveletPacket::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 获取小波滤波器系数 */
void WaveletPacket::getFilters(
    WaveletType type,
    QVector<double>& lowDec, QVector<double>& highDec,
    QVector<double>& lowRec, QVector<double>& highRec) const
{
    /* Haar小波滤波器 (长度2) */
    static const double haarLd[] = {0.7071067811865476, 0.7071067811865476};
    static const double haarHd[] = {0.7071067811865476, -0.7071067811865476};
    static const double haarLr[] = {0.7071067811865476, 0.7071067811865476};
    static const double haarHr[] = {0.7071067811865476, -0.7071067811865476};

    /* DB2小波滤波器 (长度4) */
    static const double db2Ld[] = {
        -0.1294095225501269, 0.2241438680418575,
         0.836516303737469,  0.4829629131446906
    };
    static const double db2Hd[] = {
        -0.4829629131446906, 0.836516303737469,
        -0.2241438680418575, -0.1294095225501269
    };
    static const double db2Lr[] = {
         0.4829629131446906,  0.836516303737469,
         0.2241438680418575, -0.1294095225501269
    };
    static const double db2Hr[] = {
        -0.1294095225501269, -0.2241438680418575,
         0.836516303737469,  -0.4829629131446906
    };

    /* DB4小波滤波器 (长度8) */
    static const double db4Ld[] = {
        -0.0105974017849973, 0.0328830116669829,
         0.0308413818359868, -0.1870348117188852,
        -0.0279837694169838, 0.6308807679295904,
         0.7148465705525415, 0.2303778133088964
    };
    static const double db4Hd[] = {
        -0.2303778133088964, 0.7148465705525415,
        -0.6308807679295904, -0.0279837694169838,
         0.1870348117188852, 0.0308413818359868,
        -0.0328830116669829, -0.0105974017849973
    };
    static const double db4Lr[] = {
         0.2303778133088964,  0.7148465705525415,
         0.6308807679295904, -0.0279837694169838,
        -0.1870348117188852,  0.0308413818359868,
         0.0328830116669829, -0.0105974017849973
    };
    static const double db4Hr[] = {
        -0.0105974017849973, -0.0328830116669829,
         0.0308413818359868,  0.1870348117188852,
        -0.0279837694169838, -0.6308807679295904,
         0.7148465705525415, -0.2303778133088964
    };

    switch (type) {
    case WaveletType::Haar:
        lowDec  = QVector<double>(std::begin(haarLd), std::end(haarLd));
        highDec = QVector<double>(std::begin(haarHd), std::end(haarHd));
        lowRec  = QVector<double>(std::begin(haarLr), std::end(haarLr));
        highRec = QVector<double>(std::begin(haarHr), std::end(haarHr));
        break;
    case WaveletType::DB2:
        lowDec  = QVector<double>(std::begin(db2Ld), std::end(db2Ld));
        highDec = QVector<double>(std::begin(db2Hd), std::end(db2Hd));
        lowRec  = QVector<double>(std::begin(db2Lr), std::end(db2Lr));
        highRec = QVector<double>(std::begin(db2Hr), std::end(db2Hr));
        break;
    case WaveletType::DB4:
        lowDec  = QVector<double>(std::begin(db4Ld), std::end(db4Ld));
        highDec = QVector<double>(std::begin(db4Hd), std::end(db4Hd));
        lowRec  = QVector<double>(std::begin(db4Lr), std::end(db4Lr));
        highRec = QVector<double>(std::begin(db4Hr), std::end(db4Hr));
        break;
    }
}

/** @brief 卷积运算 */
QVector<double> WaveletPacket::convolve(
    const QVector<double>& signal,
    const QVector<double>& filter) const
{
    int n = signal.size();
    int f = filter.size();
    QVector<double> result(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < f; ++j) {
            int idx = (i + j) % n;
            sum += signal[idx] * filter[j];
        }
        result[i] = sum;
    }
    return result;
}

/** @brief 计算节点能量 */
double WaveletPacket::computeEnergy(const QVector<double>& coeffs) const
{
    double energy = 0.0;
    for (double c : coeffs) {
        energy += c * c;
    }
    return energy;
}
