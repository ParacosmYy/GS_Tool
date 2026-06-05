/**
 * @file EyeDiagramEngine.cpp
 * @brief 眼图分析引擎实现 — 构造/参数/数据输入/渲染/测量/掩模
 *
 * 实现 EyeDiagramEngine 的核心功能:
 *   - setParameters(): 初始化直方图维度
 *   - feedSamples() + recoverClock(): 累积采样→边沿检测→时钟恢复
 *   - renderEyeDiagram(): 2D密度直方图→QImage彩色渲染
 *   - measure(): 眼高/眼宽/抖动/SNR/BER/Q因子计算
 *   - setMask()/maskHitRate()/loadMask(): 掩模合规测试
 *
 * 统计接口见 EyeDiagramEngineStats.cpp。
 */

#include "chart/eye/EyeDiagramEngine.h"

#include <QtMath>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

// ============================================================
// 构造 / 参数
// ============================================================

/** @brief 构造眼图引擎 @param parent 父对象 */
EyeDiagramEngine::EyeDiagramEngine(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置眼图生成参数并重建直方图 @param params 参数集 */
void EyeDiagramEngine::setParameters(const EyeParams& params)
{
    m_params = params;

    /* 计算UI周期内的采样点数(至少2个采样/周期) */
    const double samplesPerBit = m_params.sampleRate / m_params.bitRate;
    m_histCols = qMax(4, static_cast<int>(qRound(samplesPerBit * 2.0)));

    /* 电压量化级数(256级) */
    m_histRows = 256;

    /* 重建直方图 */
    m_histogram.clear();
    m_histogram.resize(m_histCols);
    for (auto& col : m_histogram) {
        col.resize(m_histRows, 0);
    }
    m_maxDensity = 1;
}

/** @brief 获取当前眼图参数 @return 参数集 */
EyeParams EyeDiagramEngine::parameters() const
{
    return m_params;
}

// ============================================================
// 数据输入
// ============================================================

/** @brief 输入采样数据，执行时钟恢复和眼图叠加 @param samples 电压采样序列 */
void EyeDiagramEngine::feedSamples(const QVector<double>& samples)
{
    if (samples.isEmpty() || m_histCols <= 0) {
        return;
    }

    m_sampleBuffer.append(samples);
    m_totalSamplesProcessed += static_cast<quint64>(samples.size());

    /* 需要至少2个UI周期的数据才进行叠加 */
    const double samplesPerBit = m_params.sampleRate / m_params.bitRate;
    const int minRequired = static_cast<int>(samplesPerBit * 2.5);
    if (m_sampleBuffer.size() < minRequired) {
        return;
    }

    /* 时钟恢复 */
    const QVector<int> edges = recoverClock(m_sampleBuffer);
    if (edges.size() < 2) {
        m_sampleBuffer.clear();
        return;
    }

    /* 按UI周期切片叠加到直方图 */
    const int sliceLen = static_cast<int>(qRound(samplesPerBit));
    const double vRange = m_voltageMax - m_voltageMin;

    for (int i = 0; i + sliceLen <= m_sampleBuffer.size(); ++i) {
        /* 仅在边沿位置附近开始切片 */
        bool nearEdge = false;
        for (int e : edges) {
            if (qAbs(i - e) <= sliceLen / 2) {
                nearEdge = true;
                break;
            }
        }
        if (!nearEdge && !edges.isEmpty()) {
            continue;
        }

        for (int j = 0; j < sliceLen && j < m_histCols; ++j) {
            const double voltage = m_sampleBuffer[i + j];
            const int row = static_cast<int>((voltage - m_voltageMin) / vRange
                                             * (m_histRows - 1));
            if (row >= 0 && row < m_histRows) {
                ++m_histogram[j][row];
                if (m_histogram[j][row] > m_maxDensity) {
                    m_maxDensity = m_histogram[j][row];
                }
            }
        }
        ++m_totalOverlays;
    }

    m_totalBitsProcessed += static_cast<quint64>(edges.size());

    /* 保留最后一个UI周期的数据作为重叠缓冲 */
    const int keepLen = static_cast<int>(samplesPerBit * 1.5);
    if (m_sampleBuffer.size() > keepLen) {
        m_sampleBuffer = m_sampleBuffer.mid(m_sampleBuffer.size() - keepLen);
    }

    emit diagramUpdated();
}

// ============================================================
// 时钟恢复
// ============================================================

/** @brief 从采样序列中检测信号边沿恢复时钟 @param samples 采样序列 @return 边沿位置索引集 */
QVector<int> EyeDiagramEngine::recoverClock(const QVector<double>& samples) const
{
    QVector<int> edges;
    if (samples.size() < 3) {
        return edges;
    }

    /* 计算信号中值作为判决阈值 */
    QVector<double> sorted = samples;
    std::sort(sorted.begin(), sorted.end());
    const double median = sorted[sorted.size() / 2];

    /* 检测过零点(跨阈值边沿) */
    const double hysteresis = (sorted[sorted.size() * 3 / 4]
                               - sorted[sorted.size() / 4]) * 0.1;

    bool high = samples[0] > median;
    for (int i = 1; i < samples.size(); ++i) {
        const bool nowHigh = samples[i] > (high ? median - hysteresis
                                                 : median + hysteresis);
        if (nowHigh != high) {
            edges.append(i);
            high = nowHigh;
        }
    }

    return edges;
}

// ============================================================
// 渲染
// ============================================================

/** @brief 渲染眼图为彩色QImage @param width 图像宽度 @param height 图像高度 @return 眼图图像 */
QImage EyeDiagramEngine::renderEyeDiagram(int width, int height) const
{
    if (width <= 0 || height <= 0 || m_histCols <= 0 || m_histRows <= 0) {
        return QImage();
    }

    QImage image(width, height, QImage::Format_RGB32);
    image.fill(Qt::black);

    if (m_maxDensity <= 1) {
        return image;
    }

    /* 逐像素映射: 像素(col,row) → 直方图bin → 归一化密度 → 颜色 */
    for (int px = 0; px < width; ++px) {
        const int histCol = static_cast<int>(
            static_cast<double>(px) / width * m_histCols);
        if (histCol < 0 || histCol >= m_histCols) {
            continue;
        }

        for (int py = 0; py < height; ++py) {
            /* Y轴翻转: 像素0=top=高电压 */
            const int histRow = static_cast<int>(
                (1.0 - static_cast<double>(py) / height) * (m_histRows - 1));
            if (histRow < 0 || histRow >= m_histRows) {
                continue;
            }

            const quint64 density = m_histogram[histCol][histRow];
            if (density > 0) {
                const double norm = qLn(1.0 + density)
                                    / qLn(1.0 + m_maxDensity);
                image.setPixelColor(px, py, densityColor(norm));
            }
        }
    }

    return image;
}

/** @brief 将归一化密度映射为颜色(蓝→绿→黄→红渐变) @param density 归一化密度(0~1) @return 颜色 */
QColor EyeDiagramEngine::densityColor(double density) const
{
    density = qBound(0.0, density, 1.0);

    int r = 0, g = 0, b = 0;
    if (density < 0.25) {
        /* 黑→蓝 */
        const double t = density / 0.25;
        b = static_cast<int>(t * 255);
    } else if (density < 0.5) {
        /* 蓝→绿 */
        const double t = (density - 0.25) / 0.25;
        g = static_cast<int>(t * 255);
        b = static_cast<int>((1.0 - t) * 255);
    } else if (density < 0.75) {
        /* 绿→黄 */
        const double t = (density - 0.5) / 0.25;
        r = static_cast<int>(t * 255);
        g = 255;
    } else {
        /* 黄→红 */
        const double t = (density - 0.75) / 0.25;
        r = 255;
        g = static_cast<int>((1.0 - t) * 255);
    }

    return QColor(r, g, b);
}

// ============================================================
// 测量
// ============================================================

/** @brief 在眼图直方图上执行信号质量测量 @return 测量结果 */
EyeMeasurement EyeDiagramEngine::measure() const
{
    EyeMeasurement m;
    if (m_histCols <= 0 || m_histRows <= 0 || m_maxDensity <= 1) {
        return m;
    }

    const double vRange = m_voltageMax - m_voltageMin;
    const double vStep = vRange / m_histRows;

    /* --- 眼高度: 在最佳采样点(中心列)计算逻辑1均值 - 逻辑0均值 --- */
    const int centerCol = m_histCols / 2;

    /* 搜索上下半区的峰值密度行 */
    int peakRowUpper = m_histRows / 2;
    quint64 maxDensityUpper = 0;
    for (int r = m_histRows / 2; r < m_histRows; ++r) {
        if (m_histogram[centerCol][r] > maxDensityUpper) {
            maxDensityUpper = m_histogram[centerCol][r];
            peakRowUpper = r;
        }
    }

    int peakRowLower = m_histRows / 2;
    quint64 maxDensityLower = 0;
    for (int r = 0; r < m_histRows / 2; ++r) {
        if (m_histogram[centerCol][r] > maxDensityLower) {
            maxDensityLower = m_histogram[centerCol][r];
            peakRowLower = r;
        }
    }

    const double vUpper = m_voltageMin + peakRowUpper * vStep;
    const double vLower = m_voltageMin + peakRowLower * vStep;
    m.eyeHeight = vUpper - vLower;
    m.eyeAmplitude = vUpper - vLower;

    /* --- 眼宽度: 在电压中点行，找到两个密度谷值之间的距离 --- */
    const int midRow = (peakRowUpper + peakRowLower) / 2;
    quint64 minDensityMid = m_histogram[0][midRow];
    int firstValley = 0, lastValley = 0;
    const quint64 threshold = m_maxDensity / 10;

    for (int c = 0; c < m_histCols; ++c) {
        if (m_histogram[c][midRow] < threshold) {
            firstValley = c;
            break;
        }
    }
    for (int c = m_histCols - 1; c >= 0; --c) {
        if (m_histogram[c][midRow] < threshold) {
            lastValley = c;
            break;
        }
    }
    if (lastValley > firstValley) {
        const double timePerSample = 1.0 / m_params.sampleRate;
        m.eyeWidth = (lastValley - firstValley) * timePerSample;
    }

    /* --- 抖动: 在交叉点区域(电压中点)测量时间展宽 --- */
    double jitterSum = 0.0;
    double jitterMax = 0.0;
    double jitterMin = 1.0e9;
    int jitterCount = 0;

    for (int c = 0; c < m_histCols; ++c) {
        if (m_histogram[c][midRow] > m_maxDensity / 4) {
            const double t = c / m_params.sampleRate;
            jitterSum += t;
            jitterMax = qMax(jitterMax, t);
            jitterMin = qMin(jitterMin, t);
            ++jitterCount;
        }
    }
    if (jitterCount > 1) {
        m.jitterPp = jitterMax - jitterMin;
        const double mean = jitterSum / jitterCount;
        double sqSum = 0.0;
        for (int c = 0; c < m_histCols; ++c) {
            if (m_histogram[c][midRow] > m_maxDensity / 4) {
                const double dt = c / m_params.sampleRate - mean;
                sqSum += dt * dt;
            }
        }
        m.jitterRms = qSqrt(sqSum / jitterCount);
    }

    /* --- 上升/下降时间 (20%~80%) --- */
    const double v20 = vLower + 0.2 * (vUpper - vLower);
    const double v80 = vLower + 0.8 * (vUpper - vLower);
    const int row20 = qBound(0, static_cast<int>((v20 - m_voltageMin) / vRange
                                                   * (m_histRows - 1)), m_histRows - 1);
    const int row80 = qBound(0, static_cast<int>((v80 - m_voltageMin) / vRange
                                                   * (m_histRows - 1)), m_histRows - 1);
    const double timePerSample = 1.0 / m_params.sampleRate;
    m.riseTime = qAbs(row80 - row20) * vStep / vRange * m_histCols * timePerSample;
    m.fallTime = m.riseTime;

    /* --- SNR (dB) --- */
    if (m.jitterRms > 0.0) {
        m.signalToNoiseRatio = 20.0 * qLog10(qMax(1.0e-12,
                                                    m.eyeHeight / (2.0 * m.jitterRms)));
    }

    /* --- Q因子 --- */
    m.qualityFactor = m.eyeHeight / (m.jitterRms > 0.0 ? (2.0 * m.jitterRms) : 1.0);

    /* --- BER估计 (基于Q因子的高斯近似) --- */
    if (m.qualityFactor > 0.0) {
        const double q = m.qualityFactor / qSqrt(2.0);
        m.bitErrorRate = qBound(1.0e-18, qExp(-q * q) / (q * qSqrt(M_PI)), 1.0);
    }

    const_cast<EyeDiagramEngine*>(this)->m_totalMeasurements++;
    emit const_cast<EyeDiagramEngine*>(this)->measurementComplete(m);

    return m;
}

// ============================================================
// 掩模测试
// ============================================================

/** @brief 设置眼图掩模 @param mask 掩模定义 */
void EyeDiagramEngine::setMask(const EyeMask& mask)
{
    m_mask = mask;
}

/** @brief 获取当前掩模 @return 掩模定义 */
EyeMask EyeDiagramEngine::mask() const
{
    return m_mask;
}

/** @brief 在直方图上执行掩模测试并返回违规率 @return 违规率(0.0~1.0) */
double EyeDiagramEngine::maskHitRate() const
{
    if (m_totalMaskTests == 0) {
        return 0.0;
    }
    return static_cast<double>(m_totalMaskHits) / m_totalMaskTests;
}

/** @brief 从JSON文件加载掩模定义 @param filePath JSON文件路径 @return 是否成功 */
bool EyeDiagramEngine::loadMask(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return false;
    }

    const QJsonObject root = doc.object();
    EyeMask loaded;
    loaded.name = root["name"].toString();

    const QJsonArray upper = root["upperBoundary"].toArray();
    for (const QJsonValue& v : upper) {
        const QJsonObject pt = v.toObject();
        loaded.upperBoundary.append(
            QPointF(pt["x"].toDouble(), pt["y"].toDouble()));
    }

    const QJsonArray lower = root["lowerBoundary"].toArray();
    for (const QJsonValue& v : lower) {
        const QJsonObject pt = v.toObject();
        loaded.lowerBoundary.append(
            QPointF(pt["x"].toDouble(), pt["y"].toDouble()));
    }

    m_mask = loaded;
    return true;
}

/** @brief 在直方图上测试掩模违规 @return 违规采样点数 */
quint64 EyeDiagramEngine::testMaskOnHistogram() const
{
    if (m_mask.upperBoundary.isEmpty() && m_mask.lowerBoundary.isEmpty()) {
        return 0;
    }

    const double vRange = m_voltageMax - m_voltageMin;
    const double tRange = m_histCols / m_params.sampleRate;
    quint64 hits = 0;

    for (int c = 0; c < m_histCols; ++c) {
        const double t = c / m_params.sampleRate;

        /* 计算掩模在当前时间点的上边界电压 */
        double upperV = m_voltageMax;
        for (int i = 0; i + 1 < m_mask.upperBoundary.size(); ++i) {
            if (t >= m_mask.upperBoundary[i].x()
                && t <= m_mask.upperBoundary[i + 1].x()) {
                const double frac = (t - m_mask.upperBoundary[i].x())
                    / (m_mask.upperBoundary[i + 1].x() - m_mask.upperBoundary[i].x());
                upperV = m_mask.upperBoundary[i].y()
                    + frac * (m_mask.upperBoundary[i + 1].y()
                              - m_mask.upperBoundary[i].y());
                break;
            }
        }

        /* 计算掩模在当前时间点的下边界电压 */
        double lowerV = m_voltageMin;
        for (int i = 0; i + 1 < m_mask.lowerBoundary.size(); ++i) {
            if (t >= m_mask.lowerBoundary[i].x()
                && t <= m_mask.lowerBoundary[i + 1].x()) {
                const double frac = (t - m_mask.lowerBoundary[i].x())
                    / (m_mask.lowerBoundary[i + 1].x() - m_mask.lowerBoundary[i].x());
                lowerV = m_mask.lowerBoundary[i].y()
                    + frac * (m_mask.lowerBoundary[i + 1].y()
                              - m_mask.lowerBoundary[i].y());
                break;
            }
        }

        /* 检查掩模区域内的直方图bin */
        const int upperRow = qBound(0, static_cast<int>(
            (upperV - m_voltageMin) / vRange * (m_histRows - 1)), m_histRows - 1);
        const int lowerRow = qBound(0, static_cast<int>(
            (lowerV - m_voltageMin) / vRange * (m_histRows - 1)), m_histRows - 1);

        for (int r = lowerRow; r <= upperRow; ++r) {
            hits += m_histogram[c][r];
        }
    }

    return hits;
}
