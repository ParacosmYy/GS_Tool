/**
 * @file DataWindowManager.cpp
 * @brief 数据窗函数引擎核心实现 -- 7种窗函数系数生成与数据加窗
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 Rectangular/Hanning/Hamming/Blackman/Kaiser/FlatTop/Gaussian
 * 七种窗函数的系数生成、逐元素加窗、相干增益计算等功能。
 * besselI0() 辅助函数用于 Kaiser 窗的修正贝塞尔函数计算。
 */

#include "utils/window/DataWindowManager.h"

#include <QElapsedTimer>
#include <QtMath>

// ═══════════════════════════════════════════════════════════════════════════════
// 构造函数
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 构造窗函数引擎
 * @param parent 父对象,纳入QObject父子树自动管理生命周期
 *
 * 默认配置: Rectangular窗, 窗长度256点。
 */
DataWindowManager::DataWindowManager(QObject *parent)
    : QObject(parent)
    , m_windowType(WindowType::Rectangular)
    , m_windowSize(256)
{
}

// ═══════════════════════════════════════════════════════════════════════════════
// 配置接口
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 设置窗函数类型
 * @param type 窗函数类型枚举值
 *
 * 设置后不会自动重新生成系数,需要调用 generateWindow() 生效。
 */
void DataWindowManager::setWindowType(WindowType type)
{
    m_windowType = type;
}

/**
 * @brief 设置窗长度
 * @param size 窗长度(采样点数),必须 > 0;无效值被忽略
 *
 * 设置后不会自动重新生成系数,需要调用 generateWindow() 生效。
 */
void DataWindowManager::setWindowSize(int size)
{
    if (size > 0) {
        m_windowSize = size;
    }
}

// ═══════════════════════════════════════════════════════════════════════════════
// 核心: 窗系数生成
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 根据当前类型和长度生成窗函数系数
 * @return 窗系数向量;窗长度 <= 0 时返回空向量
 *
 * 各窗函数公式:
 * - Rectangular: w[n] = 1.0
 * - Hanning:     w[n] = 0.5 * (1 - cos(2*PI*n/(N-1)))
 * - Hamming:     w[n] = 0.54 - 0.46 * cos(2*PI*n/(N-1))
 * - Blackman:    w[n] = 0.42 - 0.5*cos(2*PI*n/(N-1)) + 0.08*cos(4*PI*n/(N-1))
 * - Kaiser:      w[n] = I0(beta*sqrt(1-((n-(N-1)/2)/((N-1)/2))^2)) / I0(beta)
 *                 beta = 8.0
 * - FlatTop:     w[n] = a0 - a1*cos(t) + a2*cos(2t) - a3*cos(3t) + a4*cos(4t)
 *                 a0=0.21557895 a1=0.41663158 a2=0.277263158 a3=0.083578947 a4=0.006947368
 * - Gaussian:    w[n] = exp(-0.5*((n-(N-1)/2)/(sigma*(N-1)/2))^2)
 *                 sigma = 0.4
 */
QVector<double> DataWindowManager::generateWindow()
{
    if (m_windowSize <= 0) {
        return {};
    }

    const int N = m_windowSize;
    QVector<double> coeffs(N);
    const double Nm1 = qMax(static_cast<double>(N - 1), 1.0);

    switch (m_windowType) {
    case WindowType::Rectangular:
        /* 矩形窗: 所有系数为1.0 */
        for (int n = 0; n < N; ++n) {
            coeffs[n] = 1.0;
        }
        break;

    case WindowType::Hanning:
        /* 汉宁窗: 升余弦窗,旁瓣衰减约31dB */
        for (int n = 0; n < N; ++n) {
            coeffs[n] = 0.5 * (1.0 - qCos(2.0 * M_PI * n / Nm1));
        }
        break;

    case WindowType::Hamming:
        /* 海明窗: 优化升余弦窗,旁瓣衰减约42dB */
        for (int n = 0; n < N; ++n) {
            coeffs[n] = 0.54 - 0.46 * qCos(2.0 * M_PI * n / Nm1);
        }
        break;

    case WindowType::Blackman:
        /* 布莱克曼窗: 三项余弦和,旁瓣衰减约58dB */
        for (int n = 0; n < N; ++n) {
            const double t = 2.0 * M_PI * n / Nm1;
            coeffs[n] = 0.42 - 0.5 * qCos(t) + 0.08 * qCos(2.0 * t);
        }
        break;

    case WindowType::Kaiser: {
        /* 凯塞窗: 可调旁瓣,通过beta参数控制主瓣宽度与旁瓣衰减的权衡 */
        static constexpr double BETA = 8.0;  ///< Kaiser窗beta参数,8.0约对应-58dB旁瓣
        const double i0Beta = besselI0(BETA);
        const double halfN = Nm1 / 2.0;

        for (int n = 0; n < N; ++n) {
            const double ratio = (n - halfN) / halfN;
            const double arg = BETA * qSqrt(qMax(1.0 - ratio * ratio, 0.0));
            coeffs[n] = besselI0(arg) / i0Beta;
        }
        break;
    }

    case WindowType::FlatTop: {
        /* 平顶窗: 五项余弦和,主瓣最宽但幅度精度最高,适用于校准 */
        static constexpr double A0 = 0.21557895;
        static constexpr double A1 = 0.41663158;
        static constexpr double A2 = 0.277263158;
        static constexpr double A3 = 0.083578947;
        static constexpr double A4 = 0.006947368;

        for (int n = 0; n < N; ++n) {
            const double t = 2.0 * M_PI * n / Nm1;
            coeffs[n] = A0
                      - A1 * qCos(t)
                      + A2 * qCos(2.0 * t)
                      - A3 * qCos(3.0 * t)
                      + A4 * qCos(4.0 * t);
        }
        break;
    }

    case WindowType::Gaussian: {
        /* 高斯窗: 可调宽度高斯曲线,通过sigma控制主瓣宽度 */
        static constexpr double SIGMA = 0.4;  ///< 高斯窗标准差参数
        const double halfN = Nm1 / 2.0;
        const double denom = SIGMA * halfN;

        for (int n = 0; n < N; ++n) {
            const double diff = (n - halfN) / qMax(denom, 1e-15);
            coeffs[n] = qExp(-0.5 * diff * diff);
        }
        break;
    }
    }

    /* 保存到内部缓存 */
    m_coefficients = coeffs;

    /* 更新统计 */
    ++m_stats.totalWindowGenerations;

    return coeffs;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 核心: 数据加窗
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 对输入数据应用当前窗函数(逐元素相乘)
 * @param data 输入时域数据
 * @return 加窗后的数据;输入为空或长度不匹配且重新生成失败时返回空向量
 *
 * 若数据长度与当前窗系数长度不一致,自动按数据长度重新生成窗系数。
 * 更新 totalWindowsApplied / totalPointsProcessed 统计,
 * 并发射 windowApplied 信号。
 */
QVector<double> DataWindowManager::applyWindow(const QVector<double> &data)
{
    if (data.isEmpty()) {
        return {};
    }

    const int dataSize = data.size();

    /* 窗系数长度与数据不匹配时自动重新生成 */
    if (m_coefficients.size() != dataSize) {
        const int savedSize = m_windowSize;
        m_windowSize = dataSize;
        generateWindow();
        m_windowSize = savedSize;
    }

    /* 二次校验: 生成后长度仍不匹配则返回空 */
    if (m_coefficients.size() != dataSize) {
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    /* 逐元素相乘 */
    QVector<double> result(dataSize);
    for (int i = 0; i < dataSize; ++i) {
        result[i] = data[i] * m_coefficients[i];
    }

    /* 计算本次相干增益 */
    const double gain = coherentGain();

    /* 更新统计 */
    ++m_stats.totalWindowsApplied;
    m_stats.totalPointsProcessed += static_cast<quint64>(dataSize);
    if (gain > m_stats.peakCoherentGain) {
        m_stats.peakCoherentGain = gain;
    }

    emit windowApplied(dataSize);
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 查询接口
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 获取当前窗函数系数
 * @return 最近一次 generateWindow() 的结果副本
 */
QVector<double> DataWindowManager::windowCoefficients() const
{
    return m_coefficients;
}

/**
 * @brief 计算当前窗函数的相干增益
 * @return 相干增益值,定义为 sum(coeffs) / N;无系数时返回0.0
 *
 * 相干增益表示窗函数对信号直流分量的衰减程度:
 * - 矩形窗: 增益为1.0(无衰减)
 * - Hanning: 增益约0.5
 * - Hamming: 增益约0.54
 * - Blackman: 增益约0.42
 */
double DataWindowManager::coherentGain() const
{
    if (m_coefficients.isEmpty()) {
        return 0.0;
    }

    double sum = 0.0;
    for (const double c : m_coefficients) {
        sum += c;
    }

    return sum / static_cast<double>(m_coefficients.size());
}

/**
 * @brief 获取累计统计信息快照
 * @return 统计结构体副本
 */
DataWindowManager::Stats DataWindowManager::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器归零
 */
void DataWindowManager::resetStatistics()
{
    m_stats = Stats{};
}

// ═══════════════════════════════════════════════════════════════════════════════
// 辅助: 修正贝塞尔函数 I0(x)
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算修正贝塞尔函数 I0(x) 的近似值
 * @param x 输入值(非负实数)
 * @return I0(x) 近似值
 *
 * 使用幂级数展开:
 *   I0(x) = sum_{k=0}^{inf} ( (x/2)^k / k! )^2
 *
 * 收敛条件: 当新加入项的绝对值小于 1e-12 时截断。
 * 对于 Kaiser 窗常用的 beta 范围 (0~30),精度优于 1e-10。
 */
double DataWindowManager::besselI0(double x)
{
    const double halfX = x / 2.0;
    double sum = 1.0;       ///< 累计和,k=0时为1.0
    double term = 1.0;      ///< 当前项值
    static constexpr int MAX_ITERATIONS = 100;  ///< 最大迭代次数,防止无限循环

    for (int k = 1; k < MAX_ITERATIONS; ++k) {
        term *= halfX / static_cast<double>(k);
        const double termSq = term * term;
        sum += termSq;

        /* 收敛判断: 新增项足够小时提前退出 */
        if (termSq < 1e-12) {
            break;
        }
    }

    return sum;
}
