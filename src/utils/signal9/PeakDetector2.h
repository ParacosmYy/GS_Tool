/**
 * @file PeakDetector2.h
 * @brief 多准则峰值检测器 — 突出度与宽度分析
 *
 * 功能: 多准则峰值检测, 结合幅度阈值、突出度(prominence)、宽度(width)三个维度,
 *       提供精确的峰值特征分析。支持噪声鲁棒检测、峰值分类和基线校正。
 *
 * 协作: SpectrumAnalyzer(频谱峰值) / WaveformEngine(波形特征) / HeartRateMonitor(心电图R波)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 峰值描述结构
 */
struct PeakInfo {
    int index = 0;                          ///< 峰值位置(样本索引)
    double value = 0.0;                     ///< 峰值幅度
    double prominence = 0.0;                ///< 突出度(相对基线高度)
    double width = 0.0;                     ///< 半高全宽(FWHM, 样本数)
    double widthHeight = 0.0;               ///< 宽度测量高度(半高处)
    int leftBase = 0;                       ///< 左基线索引
    int rightBase = 0;                      ///< 右基线索引
    double sharpness = 0.0;                 ///< 尖锐度(峰值曲率)
    double area = 0.0;                      ///< 峰面积(三角近似)
    bool isSignificant = false;             ///< 是否为显著峰
};

/**
 * @brief 多准则峰值检测器
 *
 * 三阶段检测流程:
 * 1. 候选峰筛选: 局部极大值 + 最小幅度/距离约束
 * 2. 突出度计算: 每个峰相对其基线的高度
 * 3. 宽度分析: 半高全宽(FWHM)测量
 *
 * 每个阶段独立可配置阈值, 级联过滤保证检测质量。
 */
class PeakDetector2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 检测参数 */
    struct DetectionParams {
        double minHeight = 0.0;             ///< 最小峰高度阈值
        double minProminence = 0.0;         ///< 最小突出度阈值
        double minWidth = 0.0;              ///< 最小宽度阈值(样本)
        int minDistance = 1;                ///< 峰间最小距离(样本)
        double prominenceFraction = 0.5;    ///< 突出度分数(宽度测量高度)
        int plateauSize = 0;                ///< 平台大小容忍度
        bool sortByProminence = true;       ///< 结果按突出度排序
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalSignalsProcessed = 0;      ///< 累计处理信号数
        int totalPeaksDetected = 0;         ///< 累计检测峰值数
        int totalProminenceComputations = 0; ///< 累计突出度计算次数
        int totalWidthComputations = 0;     ///< 累计宽度计算次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit PeakDetector2(QObject* parent = nullptr);

    /**
     * @brief 检测信号中的峰值(完整流程)
     * @param signal 输入信号
     * @param params 检测参数
     * @return 峰值信息列表(按突出度降序或按位置排序)
     */
    QVector<PeakInfo> detectPeaks(const QVector<double>& signal,
                                   const DetectionParams& params = {});

    /**
     * @brief 仅查找候选局部极大值(不做突出度/宽度分析)
     * @param signal 输入信号
     * @param minDistance 峰间最小距离
     * @return 候选峰索引列表
     */
    QVector<int> findLocalMaxima(const QVector<double>& signal,
                                  int minDistance = 1) const;

    /**
     * @brief 计算指定峰的突出度
     * @param signal 输入信号
     * @param peakIndex 峰值索引
     * @param leftBase 输出: 左基线索引
     * @param rightBase 输出: 右基线索引
     * @return 突出度值
     */
    double computeProminence(const QVector<double>& signal, int peakIndex,
                              int* leftBase = nullptr, int* rightBase = nullptr) const;

    /**
     * @brief 计算指定峰的宽度(FWHM)
     * @param signal 输入信号
     * @param peakIndex 峰值索引
     * @param prominence 峰突出度
     * @param relHeight 相对高度(0~1, 默认0.5=半高)
     * @return 宽度(样本数), 包含左右插值位置
     */
    double computeWidth(const QVector<double>& signal, int peakIndex,
                         double prominence, double relHeight = 0.5) const;

    /**
     * @brief 对峰值按突出度排序
     * @param peaks 峰值列表
     * @param descending 是否降序
     * @return 排序后的峰值列表
     */
    QVector<PeakInfo> sortByProminence(const QVector<PeakInfo>& peaks,
                                        bool descending = true) const;

    /**
     * @brief 过滤不显著的峰
     * @param peaks 峰值列表
     * @param minProminence 最小突出度
     * @param minWidth 最小宽度
     * @return 过滤后的峰值列表
     */
    QVector<PeakInfo> filterPeaks(const QVector<PeakInfo>& peaks,
                                   double minProminence, double minWidth = 0.0) const;

    Stats stats() const;
    void resetStatistics();

private:
    /**
     * @brief 抑制非极大值(按最小距离)
     * @param candidates 候选峰索引
     * @param signal 输入信号
     * @param minDistance 最小距离
     * @return 抑制后的候选峰
     */
    QVector<int> suppressNonMaxima(const QVector<int>& candidates,
                                    const QVector<double>& signal,
                                    int minDistance) const;

    /**
     * @brief 在峰值左侧找基线
     * @param signal 输入信号
     * @param peakIndex 峰值索引
     * @return 左基线索引
     */
    int findLeftBase(const QVector<double>& signal, int peakIndex) const;

    /**
     * @brief 在峰值右侧找基线
     * @param signal 输入信号
     * @param peakIndex 峰值索引
     * @return 右基线索引
     */
    int findRightBase(const QVector<double>& signal, int peakIndex) const;

    /**
     * @brief 线性插值找水平线交叉点
     * @param x1 起点
     * @param y1 起点值
     * @param x2 终点
     * @param y2 终点值
     * @param level 目标水平线
     * @return 插值位置(分数索引)
     */
    double interpolateCrossing(int x1, double y1, int x2, double y2,
                                double level) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
