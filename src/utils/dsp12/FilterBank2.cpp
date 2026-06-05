/**
 * @file FilterBank2.cpp
 * @brief 二进滤波器组实现 — 小波包分解与重构
 */

#include "utils/dsp12/FilterBank2.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
FilterBank2::FilterBank2(QObject* parent)
    : QObject(parent)
    , m_filterType(FilterType::Haar)
{
    initFilterCoeffs();
}

/** @brief 设置滤波器类型 @param type 滤波器类型 */
void FilterBank2::setFilterType(FilterType type)
{
    m_filterType = type;
    initFilterCoeffs();
}

/**
 * @brief 初始化滤波器系数
 *
 * Haar: h = [1/sqrt(2), 1/sqrt(2)], g = [1/sqrt(2), -1/sqrt(2)]
 * DB2: Daubechies-2 (长度 4)
 * DB4: Daubechies-4 (长度 8)
 */
void FilterBank2::initFilterCoeffs()
{
    double invSqrt2 = 1.0 / qSqrt(2.0);

    switch (m_filterType) {
    case FilterType::Haar:
        /* 分解系数 */
        m_lowDec = {invSqrt2, invSqrt2};
        m_highDec = {invSqrt2, -invSqrt2};
        /* 重构系数 (Haar 自对称) */
        m_lowRec = {invSqrt2, invSqrt2};
        m_highRec = {invSqrt2, -invSqrt2};
        break;

    case FilterType::DB2: {
        /* Daubechies-2 (D4) 滤波器系数 */
        double c0 = (1.0 + qSqrt(3.0)) / (4.0 * qSqrt(2.0));
        double c1 = (3.0 + qSqrt(3.0)) / (4.0 * qSqrt(2.0));
        double c2 = (3.0 - qSqrt(3.0)) / (4.0 * qSqrt(2.0));
        double c3 = (1.0 - qSqrt(3.0)) / (4.0 * qSqrt(2.0));

        m_lowDec = {c0, c1, c2, c3};
        m_highDec = {c3, -c2, c1, -c0};
        m_lowRec = {c3, c2, c1, c0};
        m_highRec = {-c0, c1, -c2, c3};
        break;
    }

    case FilterType::DB4: {
        /* Daubechies-4 (D8) 滤波器系数 */
        static const double d8[] = {
            0.2303778133088964, 0.7148465705529154,
            0.6308807679398587, -0.0279837694168599,
            -0.1870348117190931, 0.0308413818355607,
            0.0328830116668852, -0.0105974017850690
        };
        int len = 8;
        m_lowDec.resize(len);
        m_highDec.resize(len);
        m_lowRec.resize(len);
        m_highRec.resize(len);

        for (int i = 0; i < len; ++i) {
            m_lowDec[i] = d8[i];
            m_highDec[i] = d8[len - 1 - i] * ((i % 2 == 0) ? 1.0 : -1.0);
            m_lowRec[i] = d8[len - 1 - i];
            m_highRec[i] = d8[i] * (((len - 1 - i) % 2 == 0) ? 1.0 : -1.0);
        }
        break;
    }
    }
}

/**
 * @brief 多级分解
 * @param data 输入信号
 * @param levels 分解层级数
 * @return 各级子带列表
 *
 * 每级将信号通过低通和高通滤波器后下采样 2 倍。
 * 第 k 级产生 2^k 个子带，共 2^1 + 2^2 + ... + 2^levels 个。
 */
QList<FilterBank2::Subband> FilterBank2::decompose(
    const QVector<double>& data, int levels)
{
    QElapsedTimer timer;
    timer.start();

    QList<Subband> result;
    int n = data.size();
    if (n < 2 || levels < 1) return result;

    /* 限制最大分解层数 */
    int maxLevels = 0;
    int temp = n;
    while (temp >= 2) { temp /= 2; ++maxLevels; }
    levels = qMin(levels, maxLevels - 1);

    /* 使用树结构进行小波包分解 */
    QList<QPair<int, QVector<double>>> currentLevel;
    currentLevel.append({0, data});

    for (int lev = 1; lev <= levels; ++lev) {
        QList<QPair<int, QVector<double>>> nextLevel;
        int bandIndex = 0;

        for (const auto& node : currentLevel) {
            /* 低通分解 */
            QVector<double> low = downsampleFilter(node.second, m_lowDec);
            /* 高通分解 */
            QVector<double> high = downsampleFilter(node.second, m_highDec);

            Subband lowBand, highBand;
            lowBand.level = lev;
            lowBand.index = bandIndex;
            lowBand.samples = low;
            lowBand.energy = computeEnergy(low);

            highBand.level = lev;
            highBand.index = bandIndex + 1;
            highBand.samples = high;
            highBand.energy = computeEnergy(high);

            result.append(lowBand);
            result.append(highBand);

            nextLevel.append({bandIndex, low});
            nextLevel.append({bandIndex + 1, high});
            bandIndex += 2;
        }

        currentLevel = nextLevel;
    }

    /* 计算能量占比 */
    double totalEnergy = 0.0;
    for (const auto& band : result) {
        totalEnergy += band.energy;
    }
    for (auto& band : result) {
        band.ratio = (totalEnergy > 0) ? band.energy / totalEnergy : 0.0;
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDecompositions;
    m_stats.totalLevelsProcessed += levels;
    m_stats.totalSamplesProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecompositions + m_stats.totalReconstructions);

    emit decompositionComplete(levels, result.size());
    return result;
}

/**
 * @brief 从分解结果重构信号
 * @param subbands 分解后的子带列表
 * @param originalLength 原始信号长度
 * @return 重构后的信号
 *
 * 使用逆滤波器对子带进行上采样和滤波，然后逐级合并。
 */
QVector<double> FilterBank2::reconstruct(const QList<Subband>& subbands,
                                         int originalLength)
{
    QElapsedTimer timer;
    timer.start();

    if (subbands.isEmpty() || originalLength < 2) {
        return QVector<double>(originalLength, 0.0);
    }

    /* 找到最大层级 */
    int maxLevel = 0;
    for (const auto& band : subbands) {
        maxLevel = qMax(maxLevel, band.level);
    }

    /* 从最深层级开始逐级重构 */
    QList<QPair<int, QVector<double>>> bands;
    for (const auto& band : subbands) {
        if (band.level == maxLevel) {
            bands.append({band.index, band.samples});
        }
    }

    for (int lev = maxLevel; lev > 1; --lev) {
        QList<QPair<int, QVector<double>>> parentBands;
        /* 成对合并: (0,1) -> 0, (2,3) -> 1, ... */
        for (int i = 0; i + 1 < bands.size(); i += 2) {
            /* 上采样 + 滤波 + 相加 */
            int targetLen = bands[i].second.size() * 2;
            QVector<double> lowUp = upsampleFilter(
                bands[i].second, m_lowRec, targetLen);
            QVector<double> highUp = upsampleFilter(
                bands[i + 1].second, m_highRec, targetLen);

            QVector<double> merged(targetLen);
            for (int j = 0; j < targetLen; ++j) {
                merged[j] = lowUp[j] + highUp[j];
            }

            parentBands.append({bands[i].first / 2, merged});
        }
        bands = parentBands;
    }

    /* 最终重构 */
    QVector<double> reconstructed;
    if (bands.size() >= 2) {
        int targetLen = originalLength;
        QVector<double> lowUp = upsampleFilter(bands[0].second, m_lowRec, targetLen);
        QVector<double> highUp = upsampleFilter(bands[1].second, m_highRec, targetLen);

        reconstructed.resize(targetLen);
        for (int i = 0; i < targetLen; ++i) {
            reconstructed[i] = lowUp[i] + highUp[i];
        }
    } else if (bands.size() == 1) {
        reconstructed = bands[0].second;
    }

    /* 截断到原始长度 */
    if (reconstructed.size() > originalLength) {
        reconstructed.resize(originalLength);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalReconstructions;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecompositions + m_stats.totalReconstructions);

    emit reconstructionComplete(reconstructed.size());
    return reconstructed;
}

/**
 * @brief 提取子带能量特征
 * @param subbands 分解结果
 * @return 各子带能量向量
 */
QVector<double> FilterBank2::energyFeatures(const QList<Subband>& subbands) const
{
    QVector<double> energies;
    energies.reserve(subbands.size());
    for (const auto& band : subbands) {
        energies.append(band.energy);
    }
    return energies;
}

/** @brief 获取低通分解系数 */
QVector<double> FilterBank2::lowpassCoefficients() const
{
    return m_lowDec;
}

/** @brief 获取高通分解系数 */
QVector<double> FilterBank2::highpassCoefficients() const
{
    return m_highDec;
}

/** @brief 重置统计信息 */
void FilterBank2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 下采样 + 滤波
 * @param data 输入数据
 * @param filter 滤波器系数
 * @return 滤波后下采样的结果
 */
QVector<double> FilterBank2::downsampleFilter(
    const QVector<double>& data, const QVector<double>& filter) const
{
    int n = data.size();
    int fLen = filter.size();
    int outLen = n / 2;
    QVector<double> output(outLen, 0.0);

    for (int i = 0; i < outLen; ++i) {
        double sum = 0.0;
        for (int j = 0; j < fLen; ++j) {
            int idx = 2 * i + j;
            /* 循环延拓 */
            if (idx >= n) idx -= n;
            sum += filter[j] * data[idx];
        }
        output[i] = sum;
    }

    return output;
}

/**
 * @brief 上采样 + 滤波
 * @param data 输入数据
 * @param filter 滤波器系数
 * @param outputLength 输出长度
 * @return 滤波后上采样的结果
 */
QVector<double> FilterBank2::upsampleFilter(
    const QVector<double>& data, const QVector<double>& filter,
    int outputLength) const
{
    int n = data.size();
    int fLen = filter.size();
    QVector<double> output(outputLength, 0.0);

    for (int i = 0; i < outputLength; ++i) {
        double sum = 0.0;
        for (int j = 0; j < fLen; ++j) {
            int dataIdx = (i - j) / 2;
            if ((i - j) % 2 == 0 && dataIdx >= 0 && dataIdx < n) {
                sum += filter[j] * data[dataIdx];
            }
        }
        output[i] = sum;
    }

    return output;
}

/**
 * @brief 计算信号能量
 * @param data 数据
 * @return 能量 (平方和)
 */
double FilterBank2::computeEnergy(const QVector<double>& data) const
{
    double energy = 0.0;
    for (double v : data) {
        energy += v * v;
    }
    return energy;
}
