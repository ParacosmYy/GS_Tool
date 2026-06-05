/**
 * @file AllpassFilter.h
 * @brief 全通滤波器 — 相位校正与级联延迟线
 *
 * 功能: 提供一阶和二阶全通滤波器节, 支持级联多节构成高阶全通网络,
 *       用于相位均衡、混响合成和分数延迟滤波器设计。
 *       全通滤波器幅度响应恒为1, 仅改变信号相位。
 *
 * 协作: EqualizerDesigner(相位均衡) / ReverbEngine(混响) / FractionalDelay(分数延迟)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 全通滤波器节描述
 */
struct AllpassSection {
    double coefficient = 0.0;   ///< 全通系数 a (一阶) 或 alpha (二阶)
    double delay = 0.0;         ///< 延迟样本数(用于嵌套全通结构)
    double x1 = 0.0;            ///< 前一输入(一阶)或 z^{-1} 状态
    double x2 = 0.0;            ///< z^{-2} 状态(二阶)
    double y1 = 0.0;            ///< 前一输出
    double y2 = 0.0;            ///< z^{-2} 输出状态

    /** @brief 相等比较(比较系数和延迟, 不比较状态) */
    bool operator==(const AllpassSection& other) const {
        return qFuzzyIsNull(coefficient - other.coefficient)
            && qFuzzyIsNull(delay - other.delay);
    }
    /** @brief 不等比较 */
    bool operator!=(const AllpassSection& other) const {
        return !(*this == other);
    }
};

/**
 * @brief 全通滤波器 — 相位校正与级联延迟线
 *
 * 支持一阶、二阶和嵌套全通滤波器节, 可级联构成任意阶数的全通网络。
 * 典型应用: 混响算法中的Schroeder全通节, 相位均衡器, 分数延迟近似。
 */
class AllpassFilter : public QObject
{
    Q_OBJECT

public:
    /** @brief 全通节类型 */
    enum class SectionType {
        FirstOrder,     ///< 一阶全通: H(z) = (a + z^{-1}) / (1 + a*z^{-1})
        SecondOrder,    ///< 二阶全通: H(z) = (a + b*z^{-1} + z^{-2}) / (1 + b*z^{-1} + a*z^{-2})
        Nested          ///< 嵌套全通: y[n] = -a*x[n] + x[n-D] + a*y[n-D]
    };
    Q_ENUM(SectionType)

    /** @brief 统计信息 */
    struct Stats {
        int totalSamplesProcessed = 0;      ///< 累计处理样本数
        int totalSectionsCreated = 0;       ///< 累计创建滤波器节数
        int totalFiltersReset = 0;          ///< 累计重置次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit AllpassFilter(QObject* parent = nullptr);

    /**
     * @brief 添加一阶全通节
     * @param coefficient 全通系数 a (|a| < 1 稳定)
     */
    void addFirstOrderSection(double coefficient);

    /**
     * @brief 添加二阶全通节
     * @param alpha z^{-2} 系数
     * @param beta z^{-1} 系数
     */
    void addSecondOrderSection(double alpha, double beta);

    /**
     * @brief 添加嵌套全通节(Schroeder混响结构)
     * @param coefficient 反馈系数 a
     * @param delaySamples 延迟线长度(样本数)
     */
    void addNestedSection(double coefficient, int delaySamples);

    /**
     * @brief 处理单个样本(通过所有级联节)
     * @param input 输入样本
     * @return 滤波后样本
     */
    double processSample(double input);

    /**
     * @brief 批量处理样本缓冲区
     * @param samples 输入样本数组
     * @return 滤波后样本数组
     */
    QVector<double> processBuffer(const QVector<double>& samples);

    /**
     * @brief 计算指定频率处的相位响应(弧度)
     * @param frequency 归一化频率 (0~0.5)
     * @return 相位值(弧度)
     */
    double phaseResponse(double frequency) const;

    /**
     * @brief 计算指定频率处的群延迟(样本)
     * @param frequency 归一化频率 (0~0.5)
     * @return 群延迟(样本数)
     */
    double groupDelay(double frequency) const;

    /**
     * @brief 计算完整相位响应曲线
     * @param numPoints 频率点数
     * @return 频率-相位对数组
     */
    QVector<QPair<double, double>> phaseResponseCurve(int numPoints = 512) const;

    /** @brief 重置所有节内部状态 */
    void resetState();

    /** @brief 获取当前级联节数 */
    int sectionCount() const;

    Stats stats() const;
    void resetStatistics();

private:
    /**
     * @brief 处理一阶全通节
     * @param section 滤波器节
     * @param input 输入样本
     * @return 输出样本
     */
    double processFirstOrder(AllpassSection& section, double input);

    /**
     * @brief 处理二阶全通节
     * @param section 滤波器节
     * @param input 输入样本
     * @return 输出样本
     */
    double processSecondOrder(AllpassSection& section, double input);

    /**
     * @brief 处理嵌套全通节
     * @param section 滤波器节
     * @param input 输入样本
     * @return 输出样本
     */
    double processNested(AllpassSection& section, double input);

    /** @brief 计算单节相位响应 */
    double sectionPhase(const AllpassSection& section, double freq) const;

    QVector<AllpassSection> m_sections;     ///< 级联全通节链
    QVector<QVector<double>> m_delayLines;  ///< 嵌套节延迟线
    QVector<int> m_delayWritePos;           ///< 延迟线写指针
    Stats m_stats;
    double m_timeSum = 0.0;
};
