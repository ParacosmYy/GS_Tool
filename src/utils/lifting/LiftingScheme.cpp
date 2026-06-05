/**
 * @file LiftingScheme.cpp
 * @brief 提升格式小波引擎实现
 */

#include "LiftingScheme.h"

#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

LiftingScheme::LiftingScheme(QObject* parent)
    : QObject(parent)
{
    loadBuiltinCoeffs(m_type);
}

LiftingScheme::~LiftingScheme() = default;

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

void LiftingScheme::setWaveletType(WaveletType type)
{
    m_type = type;
    if (type != Custom) {
        loadBuiltinCoeffs(type);
    }
}

LiftingScheme::WaveletType LiftingScheme::waveletType() const { return m_type; }

void LiftingScheme::setCustomCoeffs(const QVector<double>& predict,
                                     const QVector<double>& update)
{
    m_predictCoeffs = predict;
    m_updateCoeffs = update;
    m_type = Custom;
}

// ═══════════════════════════════════════════════════════════
// 浮点变换
// ═══════════════════════════════════════════════════════════

QVector<double> LiftingScheme::forward(const QVector<double>& signal, int levels)
{
    if (signal.size() < 2) {
        emit error(tr("提升格式正变换错误: 信号太短"));
        return signal;
    }

    QElapsedTimer timer;
    timer.start();

    QVector<double> result = signal;

    // 计算最大级数
    int maxLevels = 0;
    int len = result.size();
    while (len >= 2) { len /= 2; maxLevels++; }
    if (levels <= 0 || levels > maxLevels) levels = maxLevels;

    int currentLen = result.size();
    for (int lvl = 0; lvl < levels; ++lvl) {
        forwardOneLevel(result, currentLen);
        currentLen = (currentLen + 1) / 2;
    }

    m_stats.totalForward++;
    m_stats.totalSamples += static_cast<quint64>(signal.size());
    m_stats.avgTimeMs = (m_stats.avgTimeMs * (m_stats.totalForward - 1) +
                         timer.elapsed()) / static_cast<double>(m_stats.totalForward);

    emit forwardCompleted(result.size());
    return result;
}

QVector<double> LiftingScheme::inverse(const QVector<double>& coefficients,
                                        int originalLength, int levels)
{
    if (coefficients.size() < 2) return coefficients;

    QVector<double> result = coefficients;

    // 从最深层开始重构
    int maxLevels = 0;
    int len = originalLength;
    while (len >= 2) { len /= 2; maxLevels++; }
    if (levels <= 0 || levels > maxLevels) levels = maxLevels;

    for (int lvl = levels; lvl > 0; --lvl) {
        int currentLen = originalLength;
        for (int i = 0; i < lvl - 1; ++i) {
            currentLen = (currentLen + 1) / 2;
        }
        inverseOneLevel(result, currentLen);
    }

    m_stats.totalInverse++;
    if (result.size() > originalLength) result.resize(originalLength);

    emit inverseCompleted(result.size());
    return result;
}

// ═══════════════════════════════════════════════════════════
// 整数变换
// ═══════════════════════════════════════════════════════════

QVector<qint64> LiftingScheme::forwardInt(const QVector<qint64>& signal)
{
    if (signal.size() < 2) return signal;

    QVector<qint64> result = signal;
    int len = result.size();

    // 多级分解(使用CDF 5/3整数提升)
    while (len >= 2) {
        forwardOneLevelInt(result, len);
        len = (len + 1) / 2;
    }

    m_stats.totalForward++;
    m_stats.totalSamples += static_cast<quint64>(signal.size());
    return result;
}

QVector<qint64> LiftingScheme::inverseInt(const QVector<qint64>& coefficients,
                                           int originalLength)
{
    if (coefficients.size() < 2) return coefficients;

    QVector<qint64> result = coefficients;

    // 从小到大逐级重构
    int len = 2;
    while (len < originalLength) {
        inverseOneLevelInt(result, len);
        len *= 2;
    }

    if (result.size() > originalLength) result.resize(originalLength);

    m_stats.totalInverse++;
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

LiftingScheme::Stats LiftingScheme::stats() const { return m_stats; }
void LiftingScheme::resetStatistics() { m_stats = Stats{}; }

// ═══════════════════════════════════════════════════════════
// 内部: 加载内置系数
// ═══════════════════════════════════════════════════════════

void LiftingScheme::loadBuiltinCoeffs(WaveletType type)
{
    switch (type) {
    case Haar:
        m_predictCoeffs = {1.0};
        m_updateCoeffs = {0.5};
        m_predictScale = 1.0 / std::sqrt(2.0);
        m_updateScale = std::sqrt(2.0);
        break;

    case Cdf53:
        // CDF 5/3: predict = -0.5, update = 0.25
        m_predictCoeffs = {-0.5};
        m_updateCoeffs = {0.25};
        m_predictScale = 1.0;
        m_updateScale = 1.0;
        break;

    case Cdf97:
        // CDF 9/7: 4步提升
        m_predictCoeffs = {-1.586134342, -0.8829110762};
        m_updateCoeffs = {0.05298011854, 0.4435068522};
        m_predictScale = 1.149604398;
        m_updateScale = 1.0 / 1.149604398;
        break;

    default:
        m_predictCoeffs = {-0.5};
        m_updateCoeffs = {0.25};
        m_predictScale = 1.0;
        m_updateScale = 1.0;
        break;
    }
}

// ═══════════════════════════════════════════════════════════
// 内部: 单级变换
// ═══════════════════════════════════════════════════════════

void LiftingScheme::forwardOneLevel(QVector<double>& signal, int length)
{
    // Step 1: Split — 奇偶分离(就地, 交织存储)
    // Step 2: Predict — 用偶数样本预测奇数, 计算细节
    // Step 3: Update — 用细节更新偶数, 计算近似

    const int half = (length + 1) / 2;

    if (m_type == Cdf97 && m_predictCoeffs.size() >= 2) {
        // CDF 9/7: 4步提升
        const double alpha = -1.586134342;
        const double beta = -0.8829110762;
        const double gamma = 0.05298011854;
        const double delta = 0.4435068522;
        const double k = 1.149604398;

        // Predict 1
        for (int i = 1; i < length - 1; i += 2) {
            signal[i] += alpha * (signal[i - 1] + signal[i + 1]);
        }
        if (length % 2 == 0) {
            signal[length - 1] += alpha * signal[length - 2];
        }

        // Update 1
        for (int i = 2; i < length; i += 2) {
            signal[i] += beta * (signal[i - 1] + signal[i + 1 < length ? i + 1 : i - 1]);
        }
        signal[0] += beta * signal[1];

        // Predict 2
        for (int i = 1; i < length - 1; i += 2) {
            signal[i] += gamma * (signal[i - 1] + signal[i + 1]);
        }
        if (length % 2 == 0) {
            signal[length - 1] += gamma * signal[length - 2];
        }

        // Update 2
        for (int i = 2; i < length; i += 2) {
            signal[i] += delta * (signal[i - 1] + signal[i + 1 < length ? i + 1 : i - 1]);
        }
        signal[0] += delta * signal[1];

        // Scale
        for (int i = 0; i < length; i += 2) signal[i] *= (1.0 / k);
        for (int i = 1; i < length; i += 2) signal[i] *= k;
    } else {
        // 标准单步提升(Haar或CDF 5/3)
        // Predict: detail = odd - predict(even)
        for (int i = 1; i < length; i += 2) {
            double predict = 0.0;
            if (i - 1 >= 0) predict = signal[i - 1];
            if (i + 1 < length) predict = (signal[i - 1] + signal[i + 1]) * 0.5;
            signal[i] -= predict;
        }

        // Update: approx = even + update(detail)
        for (int i = 0; i < length; i += 2) {
            double update = 0.0;
            if (i - 1 >= 0) update += signal[i - 1] * 0.25;
            if (i + 1 < length) update += signal[i + 1] * 0.25;
            signal[i] += update;
        }
    }
}

void LiftingScheme::inverseOneLevel(QVector<double>& signal, int length)
{
    if (m_type == Cdf97 && m_predictCoeffs.size() >= 2) {
        const double alpha = -1.586134342;
        const double beta = -0.8829110762;
        const double gamma = 0.05298011854;
        const double delta = 0.4435068522;
        const double k = 1.149604398;

        // 反Scale
        for (int i = 0; i < length; i += 2) signal[i] *= k;
        for (int i = 1; i < length; i += 2) signal[i] *= (1.0 / k);

        // 反Update 2
        signal[0] -= delta * signal[1];
        for (int i = 2; i < length; i += 2) {
            signal[i] -= delta * (signal[i - 1] + signal[i + 1 < length ? i + 1 : i - 1]);
        }

        // 反Predict 2
        if (length % 2 == 0) signal[length - 1] -= gamma * signal[length - 2];
        for (int i = 1; i < length - 1; i += 2) {
            signal[i] -= gamma * (signal[i - 1] + signal[i + 1]);
        }

        // 反Update 1
        signal[0] -= beta * signal[1];
        for (int i = 2; i < length; i += 2) {
            signal[i] -= beta * (signal[i - 1] + signal[i + 1 < length ? i + 1 : i - 1]);
        }

        // 反Predict 1
        if (length % 2 == 0) signal[length - 1] -= alpha * signal[length - 2];
        for (int i = 1; i < length - 1; i += 2) {
            signal[i] -= alpha * (signal[i - 1] + signal[i + 1]);
        }
    } else {
        // 反Update
        for (int i = 0; i < length; i += 2) {
            double update = 0.0;
            if (i - 1 >= 0) update += signal[i - 1] * 0.25;
            if (i + 1 < length) update += signal[i + 1] * 0.25;
            signal[i] -= update;
        }

        // 反Predict
        for (int i = 1; i < length; i += 2) {
            double predict = 0.0;
            if (i - 1 >= 0) predict = signal[i - 1];
            if (i + 1 < length) predict = (signal[i - 1] + signal[i + 1]) * 0.5;
            signal[i] += predict;
        }
    }
}

void LiftingScheme::forwardOneLevelInt(QVector<qint64>& signal, int length)
{
    // CDF 5/3 整数提升
    // Predict: d[n] = odd[n] - floor((even[n] + even[n+1]) / 2)
    for (int i = 1; i < length - 1; i += 2) {
        signal[i] -= (signal[i - 1] + signal[i + 1] + 2) / 4;
    }
    if ((length & 1) == 0 && length > 1) {
        signal[length - 1] -= (signal[length - 2] + 1) / 2;
    }

    // Update: s[n] = even[n] + floor((d[n-1] + d[n]) / 4)
    for (int i = 0; i < length; i += 2) {
        if (i - 1 >= 0 && i + 1 < length) {
            signal[i] += (signal[i - 1] + signal[i + 1]) / 4;
        } else if (i + 1 < length) {
            signal[i] += signal[i + 1] / 4;
        }
    }
}

void LiftingScheme::inverseOneLevelInt(QVector<qint64>& signal, int length)
{
    // 反Update
    for (int i = 0; i < length; i += 2) {
        if (i - 1 >= 0 && i + 1 < length) {
            signal[i] -= (signal[i - 1] + signal[i + 1]) / 4;
        } else if (i + 1 < length) {
            signal[i] -= signal[i + 1] / 4;
        }
    }

    // 反Predict
    for (int i = 1; i < length - 1; i += 2) {
        signal[i] += (signal[i - 1] + signal[i + 1] + 2) / 4;
    }
    if ((length & 1) == 0 && length > 1) {
        signal[length - 1] += (signal[length - 2] + 1) / 2;
    }
}
