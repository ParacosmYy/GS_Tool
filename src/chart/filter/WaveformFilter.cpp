/**
 * @file WaveformFilter.cpp
 * @brief 波形数字滤波器核心实现 -- 构造/参数配置/分发/级联/系数/频率响应/统计
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 本文件实现: 构造/析构、setFilterParams、apply/applyInPlace、级联管理、
 *             coefficients、frequencyResponse、统计getter/reset、分发器。
 * 各滤波算法实现见 WaveformFilterCompute.cpp。
 */

#include "chart/filter/WaveformFilter.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 构造波形滤波器,设置默认objectName
 * @param parent 父对象,纳入QObject父子树自动管理生命周期
 */
WaveformFilter::WaveformFilter(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("WaveformFilter"));
}

/** @brief 析构函数 -- QObject父子树自动回收 */
WaveformFilter::~WaveformFilter() = default;

// ═══════════════════════════════════════════════════════════════════════════════
// 参数配置
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 设置主滤波器参数
 * @param type 滤波器类型
 * @param params 参数映射:
 *   LowPass/HighPass: "cutoffFreq"(Hz), "sampleRate"(Sa/s)
 *   BandPass/BandStop: "lowCutoff"(Hz), "highCutoff"(Hz), "sampleRate"(Sa/s)
 *   MovingAverage: "windowSize"(int, 默认5)
 *   Median: "windowSize"(int, 默认5, 自动取奇)
 *   Exponential: "alpha"(double, 0~1, 默认0.3)
 *   SavitzkyGolay: "windowSize"(int, 默认11), "polyOrder"(int, 默认3)
 */
void WaveformFilter::setFilterParams(FilterType type, const QVariantMap &params)
{
    m_filterType = type;
    m_filterParams = params;
}

/** @brief 获取当前主滤波器类型 */
WaveformFilter::FilterType WaveformFilter::filterType() const
{
    return m_filterType;
}

/** @brief 获取当前主滤波器参数 */
QVariantMap WaveformFilter::filterParams() const
{
    return m_filterParams;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 单次滤波
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 对数据应用当前主滤波器(不修改原数据)
 * @param data 输入采样数据
 * @return 滤波后数据;输入为空或参数无效时返回空
 */
QVector<double> WaveformFilter::apply(const QVector<double> &data)
{
    if (data.isEmpty()) {
        emit error(tr("滤波输入数据为空"));
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    QVector<double> result = dispatchFilter(m_filterType, m_filterParams, data);

    /* 更新统计 */
    const double elapsed = timer.elapsed();
    ++m_totalFiltersApplied;
    m_totalSamplesProcessed += static_cast<quint64>(data.size());
    m_totalProcessingTimeMs += elapsed;
    ++m_timeCount;
    ++m_filtersByType[static_cast<int>(m_filterType)];

    emit filterApplied(data.size(), elapsed);
    return result;
}

/**
 * @brief 就地应用主滤波器
 * @param data 输入/输出采样数据
 * @return true=成功, false=输入为空
 */
bool WaveformFilter::applyInPlace(QVector<double> &data)
{
    QVector<double> filtered = apply(data);
    if (filtered.isEmpty() && !data.isEmpty()) {
        return false;
    }
    data = std::move(filtered);
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 级联滤波
// ═══════════════════════════════════════════════════════════════════════════════

/** @brief 追加级联步骤 */
void WaveformFilter::cascadeAppend(FilterType type, const QVariantMap &params)
{
    m_cascadeChain.append({type, params});
}

/** @brief 清空级联链 */
void WaveformFilter::cascadeClear()
{
    m_cascadeChain.clear();
}

/** @brief 获取级联链 */
QVector<WaveformFilter::CascadeStep> WaveformFilter::cascadeChain() const
{
    return m_cascadeChain;
}

/**
 * @brief 按级联链依次应用所有滤波器
 * @param data 输入采样数据
 * @return 依次通过所有级联步骤后的结果
 */
QVector<double> WaveformFilter::applyCascade(const QVector<double> &data)
{
    if (data.isEmpty()) {
        emit error(tr("级联滤波输入数据为空"));
        return {};
    }
    if (m_cascadeChain.isEmpty()) {
        return data;
    }

    QElapsedTimer timer;
    timer.start();

    QVector<double> result = data;
    for (const auto &step : m_cascadeChain) {
        result = dispatchFilter(step.type, step.params, result);
        if (result.isEmpty()) {
            emit error(tr("级联滤波中间步骤产生空结果"));
            return {};
        }
    }

    /* 更新统计 */
    const double elapsed = timer.elapsed();
    ++m_totalCascadeOps;
    m_totalSamplesProcessed += static_cast<quint64>(data.size());
    m_totalProcessingTimeMs += elapsed;
    ++m_timeCount;

    emit filterApplied(data.size(), elapsed);
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 滤波器系数
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 获取主滤波器系数
 * @return 系数向量(LowPass/HighPass→[b0,a1], Exponential→[alpha,1-alpha],
 *         MovingAverage→1/N均值向量, 其他→空)
 */
QVector<double> WaveformFilter::coefficients() const
{
    switch (m_filterType) {
    case FilterType::LowPass:
    case FilterType::HighPass: {
        const double fc = m_filterParams.value(QStringLiteral("cutoffFreq"), 0.0).toDouble();
        const double sr = m_filterParams.value(QStringLiteral("sampleRate"), 1.0).toDouble();
        if (fc <= 0.0 || sr <= 0.0) {
            return {};
        }
        const double rc = 1.0 / (2.0 * M_PI * fc);
        const double dt = 1.0 / sr;
        const double alpha = dt / (rc + dt);
        return {alpha, 1.0 - alpha};
    }
    case FilterType::Exponential: {
        const double a = m_filterParams.value(QStringLiteral("alpha"), 0.3).toDouble();
        return {a, 1.0 - a};
    }
    case FilterType::MovingAverage: {
        const int ws = m_filterParams.value(QStringLiteral("windowSize"), 5).toInt();
        if (ws <= 0) {
            return {};
        }
        const double val = 1.0 / static_cast<double>(ws);
        return QVector<double>(ws, val);
    }
    default:
        return {};
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// 频率响应估计
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 估计主滤波器幅频响应
 * @param freqPoints 频率点向量(Hz)
 * @return 归一化幅度响应(0~1)
 *
 * LowPass/HighPass: 单极RC传递函数 H(f)=1/sqrt(1+(f/fc)^2)
 * MovingAverage: sinc近似 |H(f)|=|sin(pi*f*N/fs)/(N*sin(pi*f/fs))|
 * 其他: 返回全1(未实现详细估计)
 */
QVector<double> WaveformFilter::frequencyResponse(const QVector<double> &freqPoints) const
{
    if (freqPoints.isEmpty()) {
        return {};
    }

    QVector<double> response;
    response.reserve(freqPoints.size());

    switch (m_filterType) {
    case FilterType::LowPass: {
        const double fc = m_filterParams.value(QStringLiteral("cutoffFreq"), 1.0).toDouble();
        for (double f : freqPoints) {
            response.append(1.0 / std::sqrt(1.0 + (f / fc) * (f / fc)));
        }
        break;
    }
    case FilterType::HighPass: {
        const double fc = m_filterParams.value(QStringLiteral("cutoffFreq"), 1.0).toDouble();
        for (double f : freqPoints) {
            response.append((f / fc) / std::sqrt(1.0 + (f / fc) * (f / fc)));
        }
        break;
    }
    case FilterType::MovingAverage: {
        const int ws = m_filterParams.value(QStringLiteral("windowSize"), 5).toInt();
        const double sr = m_filterParams.value(QStringLiteral("sampleRate"), 1000.0).toDouble();
        const double N = static_cast<double>(ws);
        for (double f : freqPoints) {
            if (f <= 0.0) {
                response.append(1.0);
            } else {
                const double x = M_PI * f * N / sr;
                response.append(std::abs(std::sin(x) / (N * std::sin(M_PI * f / sr))));
            }
        }
        break;
    }
    case FilterType::Exponential: {
        const double alpha = m_filterParams.value(QStringLiteral("alpha"), 0.3).toDouble();
        for (double f : freqPoints) {
            /* 一阶IIR: 简化恒定增益近似 */
            Q_UNUSED(f)
            response.append(alpha);
        }
        break;
    }
    default:
        response.fill(1.0, freqPoints.size());
        break;
    }

    return response;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 分发器
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 根据滤波器类型分发到对应算法实现
 * @param type 滤波器类型
 * @param params 参数映射
 * @param data 输入数据
 * @return 滤波结果
 */
QVector<double> WaveformFilter::dispatchFilter(FilterType type, const QVariantMap &params,
                                               const QVector<double> &data)
{
    switch (type) {
    case FilterType::MovingAverage: {
        const int ws = params.value(QStringLiteral("windowSize"), 5).toInt();
        return filterMovingAverage(data, ws);
    }
    case FilterType::Median: {
        int ws = params.value(QStringLiteral("windowSize"), 5).toInt();
        if (ws % 2 == 0) {
            ++ws;
        }
        return filterMedian(data, ws);
    }
    case FilterType::Exponential: {
        double alpha = params.value(QStringLiteral("alpha"), 0.3).toDouble();
        alpha = std::clamp(alpha, 0.0, 1.0);
        return filterExponential(data, alpha);
    }
    case FilterType::LowPass: {
        const double fc = params.value(QStringLiteral("cutoffFreq"), 0.0).toDouble();
        const double sr = params.value(QStringLiteral("sampleRate"), 1.0).toDouble();
        return filterLowPass(data, fc, sr);
    }
    case FilterType::HighPass: {
        const double fc = params.value(QStringLiteral("cutoffFreq"), 0.0).toDouble();
        const double sr = params.value(QStringLiteral("sampleRate"), 1.0).toDouble();
        return filterHighPass(data, fc, sr);
    }
    case FilterType::BandPass: {
        const double low = params.value(QStringLiteral("lowCutoff"), 0.0).toDouble();
        const double high = params.value(QStringLiteral("highCutoff"), 0.0).toDouble();
        const double sr = params.value(QStringLiteral("sampleRate"), 1.0).toDouble();
        return filterBandPass(data, low, high, sr);
    }
    case FilterType::BandStop: {
        const double low = params.value(QStringLiteral("lowCutoff"), 0.0).toDouble();
        const double high = params.value(QStringLiteral("highCutoff"), 0.0).toDouble();
        const double sr = params.value(QStringLiteral("sampleRate"), 1.0).toDouble();
        return filterBandStop(data, low, high, sr);
    }
    case FilterType::SavitzkyGolay: {
        int ws = params.value(QStringLiteral("windowSize"), 11).toInt();
        const int po = params.value(QStringLiteral("polyOrder"), 3).toInt();
        if (ws % 2 == 0) {
            ++ws;
        }
        return filterSavitzkyGolay(data, ws, po);
    }
    }
    return data;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 获取统计信息快照
 * @return 当前统计数据
 */
WaveformFilter::Stats WaveformFilter::stats() const
{
    Stats s;
    s.totalFiltersApplied    = m_totalFiltersApplied;
    s.totalSamplesProcessed  = m_totalSamplesProcessed;
    s.avgProcessingTimeMs    = (m_timeCount > 0)
        ? m_totalProcessingTimeMs / static_cast<double>(m_timeCount) : 0.0;
    s.totalCascadeOperations = m_totalCascadeOps;
    s.filtersByType          = m_filtersByType;
    return s;
}

/** @brief 重置所有统计计数器为初始值 */
void WaveformFilter::resetStatistics()
{
    m_totalFiltersApplied   = 0;
    m_totalSamplesProcessed = 0;
    m_totalCascadeOps       = 0;
    m_totalProcessingTimeMs = 0.0;
    m_timeCount             = 0;
    m_filtersByType.clear();
}
