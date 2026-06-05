/**
 * @file WaveformFilter.cpp
 * @brief 波形数字滤波器核心实现 -- 构造/参数配置/分发/级联/系数/频率响应/统计
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 本文件实现: 构造/析构、setFilterParams、apply/applyInPlace、级联管理、
 *             coefficients、frequencyResponse、统计getter/reset。
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
            /* 一阶IIR幅度: |H| = alpha / sqrt(1 - 2*(1-alpha)*cos(2*pi*f/fs) + (1-alpha)^2) */
            response.append(alpha);  /* 简化: 恒定增益近似 */
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
            ++ws;  /* 中值滤波窗口必须为奇数 */
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
            ++ws;  /* SG窗口必须为奇数 */
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

// ═══════════════════════════════════════════════════════════════════════════════
// 滤波算法实现
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 滑动平均滤波
 * @param data 输入数据
 * @param windowSize 窗口大小(>=1)
 * @return 滤波后数据
 *
 * 每个输出点 = 窗口内采样值的算术平均,边界处对称填充。
 */
QVector<double> WaveformFilter::filterMovingAverage(const QVector<double> &data, int windowSize)
{
    if (data.isEmpty() || windowSize < 1) {
        return data;
    }

    const int n = data.size();
    const int half = windowSize / 2;
    QVector<double> result(n);

    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        int count = 0;
        for (int j = -half; j <= half; ++j) {
            const int idx = std::clamp(i + j, 0, n - 1);
            sum += data[idx];
            ++count;
        }
        result[i] = sum / static_cast<double>(count);
    }
    return result;
}

/**
 * @brief 中值滤波
 * @param data 输入数据
 * @param windowSize 窗口大小(奇数)
 * @return 滤波后数据
 *
 * 每个输出点 = 窗口内采样值的中位数,边界处对称填充。
 */
QVector<double> WaveformFilter::filterMedian(const QVector<double> &data, int windowSize)
{
    if (data.isEmpty() || windowSize < 1) {
        return data;
    }

    const int n = data.size();
    const int half = windowSize / 2;
    QVector<double> result(n);

    for (int i = 0; i < n; ++i) {
        QVector<double> window;
        window.reserve(windowSize);
        for (int j = -half; j <= half; ++j) {
            const int idx = std::clamp(i + j, 0, n - 1);
            window.append(data[idx]);
        }
        std::sort(window.begin(), window.end());
        result[i] = window[window.size() / 2];
    }
    return result;
}

/**
 * @brief 指数移动平均(EMA)
 * @param data 输入数据
 * @param alpha 平滑因子(0~1),值越大跟踪越快,平滑越弱
 * @return 滤波后数据
 *
 * 递推公式: y[n] = alpha * x[n] + (1-alpha) * y[n-1]
 */
QVector<double> WaveformFilter::filterExponential(const QVector<double> &data, double alpha)
{
    if (data.isEmpty()) {
        return data;
    }

    const int n = data.size();
    QVector<double> result(n);
    result[0] = data[0];

    for (int i = 1; i < n; ++i) {
        result[i] = alpha * data[i] + (1.0 - alpha) * result[i - 1];
    }
    return result;
}

/**
 * @brief 低通RC滤波(单极IIR)
 * @param data 输入数据
 * @param cutoffFreq 截止频率(Hz)
 * @param sampleRate 采样率(Sa/s)
 * @return 滤波后数据
 *
 * RC时间常数: tau = 1/(2*pi*fc), alpha = dt/(tau+dt)
 * 递推: y[n] = alpha*x[n] + (1-alpha)*y[n-1]
 */
QVector<double> WaveformFilter::filterLowPass(const QVector<double> &data, double cutoffFreq,
                                              double sampleRate)
{
    if (data.isEmpty() || cutoffFreq <= 0.0 || sampleRate <= 0.0) {
        return data;
    }

    const double rc = 1.0 / (2.0 * M_PI * cutoffFreq);
    const double dt = 1.0 / sampleRate;
    const double alpha = dt / (rc + dt);

    const int n = data.size();
    QVector<double> result(n);
    result[0] = data[0];

    for (int i = 1; i < n; ++i) {
        result[i] = alpha * data[i] + (1.0 - alpha) * result[i - 1];
    }
    return result;
}

/**
 * @brief 高通RC滤波(单极IIR)
 * @param data 输入数据
 * @param cutoffFreq 截止频率(Hz)
 * @param sampleRate 采样率(Sa/s)
 * @return 滤波后数据
 *
 * 递推: y[n] = alpha*(y[n-1] + x[n] - x[n-1])
 */
QVector<double> WaveformFilter::filterHighPass(const QVector<double> &data, double cutoffFreq,
                                               double sampleRate)
{
    if (data.isEmpty() || cutoffFreq <= 0.0 || sampleRate <= 0.0) {
        return data;
    }

    const double rc = 1.0 / (2.0 * M_PI * cutoffFreq);
    const double dt = 1.0 / sampleRate;
    const double alpha = rc / (rc + dt);

    const int n = data.size();
    QVector<double> result(n);
    result[0] = data[0];

    for (int i = 1; i < n; ++i) {
        result[i] = alpha * (result[i - 1] + data[i] - data[i - 1]);
    }
    return result;
}

/**
 * @brief 带通滤波 -- 先高通后低通级联
 * @param data 输入数据
 * @param lowCutoff 低截止频率(Hz)
 * @param highCutoff 高截止频率(Hz)
 * @param sr 采样率(Sa/s)
 * @return 带通滤波结果
 */
QVector<double> WaveformFilter::filterBandPass(const QVector<double> &data, double lowCutoff,
                                               double highCutoff, double sr)
{
    if (data.isEmpty() || lowCutoff <= 0.0 || highCutoff <= lowCutoff || sr <= 0.0) {
        return data;
    }
    /* 先通过高通去除低频,再通过低通去除高频 */
    QVector<double> hp = filterHighPass(data, lowCutoff, sr);
    return filterLowPass(hp, highCutoff, sr);
}

/**
 * @brief 带阻滤波 -- 低通+高通差分组合
 * @param data 输入数据
 * @param lowCutoff 低截止频率(Hz)
 * @param highCutoff 高截止频率(Hz)
 * @param sr 采样率(Sa/s)
 * @return 带阻滤波结果
 *
 * y = lowPass(x, lowCutoff) + highPass(x, highCutoff) - x
 * 等效于从原信号中减去带通分量。
 */
QVector<double> WaveformFilter::filterBandStop(const QVector<double> &data, double lowCutoff,
                                               double highCutoff, double sr)
{
    if (data.isEmpty() || lowCutoff <= 0.0 || highCutoff <= lowCutoff || sr <= 0.0) {
        return data;
    }
    QVector<double> lp = filterLowPass(data, lowCutoff, sr);
    QVector<double> hp = filterHighPass(data, highCutoff, sr);

    const int n = data.size();
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = lp[i] + hp[i];
    }
    return result;
}

/**
 * @brief Savitzky-Golay多项式拟合滤波
 * @param data 输入数据
 * @param windowSize 窗口大小(奇数, >= polyOrder+1)
 * @param polyOrder 多项式阶数(>=1)
 * @return 滤波后数据
 *
 * 使用最小二乘法拟合局部多项式,取中心点作为输出。
 * 边界处使用对称填充。通过法方程(J^T*J)*c = J^T*y 求解卷积系数。
 */
QVector<double> WaveformFilter::filterSavitzkyGolay(const QVector<double> &data, int windowSize,
                                                    int polyOrder)
{
    if (data.isEmpty() || windowSize < 3 || polyOrder < 1) {
        return data;
    }
    if (polyOrder >= windowSize) {
        polyOrder = windowSize - 1;
    }

    const int n = data.size();
    const int half = windowSize / 2;

    /* 构建 Vandermonde 矩阵 J[i][j] = i^j (i从-half到half) */
    const int cols = polyOrder + 1;
    QVector<QVector<double>> J(windowSize, QVector<double>(cols, 0.0));
    for (int i = 0; i < windowSize; ++i) {
        const double x = static_cast<double>(i - half);
        J[i][0] = 1.0;
        for (int j = 1; j < cols; ++j) {
            J[i][j] = J[i][j - 1] * x;
        }
    }

    /* 计算 J^T * J (cols x cols) */
    QVector<QVector<double>> JtJ(cols, QVector<double>(cols, 0.0));
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < cols; ++j) {
            double sum = 0.0;
            for (int k = 0; k < windowSize; ++k) {
                sum += J[k][i] * J[k][j];
            }
            JtJ[i][j] = sum;
        }
    }

    /* 高斯消元法求 JtJ 的逆矩阵 */
    /* 构建增广矩阵 [JtJ | I] */
    QVector<QVector<double>> aug(cols, QVector<double>(2 * cols, 0.0));
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < cols; ++j) {
            aug[i][j] = JtJ[i][j];
        }
        aug[i][cols + i] = 1.0;
    }

    /* 前向消元 */
    for (int col = 0; col < cols; ++col) {
        /* 选主元 */
        int maxRow = col;
        for (int row = col + 1; row < cols; ++row) {
            if (std::abs(aug[row][col]) > std::abs(aug[maxRow][col])) {
                maxRow = row;
            }
        }
        std::swap(aug[col], aug[maxRow]);

        const double pivot = aug[col][col];
        if (std::abs(pivot) < 1e-12) {
            return data;  /* 奇异矩阵,回退原数据 */
        }
        for (int j = 0; j < 2 * cols; ++j) {
            aug[col][j] /= pivot;
        }
        for (int row = 0; row < cols; ++row) {
            if (row == col) {
                continue;
            }
            const double factor = aug[row][col];
            for (int j = 0; j < 2 * cols; ++j) {
                aug[row][j] -= factor * aug[col][j];
            }
        }
    }

    /* 提取逆矩阵 invJtJ */
    QVector<QVector<double>> invJtJ(cols, QVector<double>(cols, 0.0));
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < cols; ++j) {
            invJtJ[i][j] = aug[i][cols + j];
        }
    }

    /* 计算卷积核: conv_coeff[k] = sum_{j=0}^{cols-1} invJtJ[0][j] * J[k][j] */
    /* 因为只需要中心点的平滑值,取多项式第0阶(常数项)系数即可 */
    QVector<double> convCoeff(windowSize, 0.0);
    for (int k = 0; k < windowSize; ++k) {
        double val = 0.0;
        for (int j = 0; j < cols; ++j) {
            val += invJtJ[0][j] * J[k][j];
        }
        convCoeff[k] = val;
    }

    /* 应用卷积核 */
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int k = 0; k < windowSize; ++k) {
            const int idx = std::clamp(i + k - half, 0, n - 1);
            sum += convCoeff[k] * data[idx];
        }
        result[i] = sum;
    }
    return result;
}
