/**
 * @file WaveformPatternDetector.h
 * @brief 波形模式检测器 -- 自动识别信号特征模式
 *
 * 设计: 纯分析类(不依赖QWidget)、基于在线统计(Welford算法)和自相关分析，
 * 自动检测周期脉冲/电平跳变/毛刺/跌落/正弦波等5种信号模式，
 * 输出置信度评分和频率估计，适合嵌入式信号质量分析。
 * 协作: 波形数据源(feedSample/feedSamples) → WaveformPatternDetector(analyze) → UI/通知
 */

#ifndef WAVEFORMPATTERNDETECTOR_H
#define WAVEFORMPATTERNDETECTOR_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QList>

/**
 * @brief 波形模式检测器 -- 在线统计与自相关分析，自动识别信号特征模式
 */
class WaveformPatternDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 检测到的模式类型 */
    enum class PatternType {
        PeriodicPulse,     ///< 周期脉冲 -- 信号以固定间隔重复出现
        LevelChange,       ///< 电平跳变 -- 信号均值发生突然偏移
        Spike,             ///< 毛刺 -- 短时大幅偏离均值的异常尖峰
        Dropout,           ///< 跌落 -- 信号突然降至接近零或极低值
        SineWave,          ///< 正弦波 -- 检测到近似正弦的周期振荡
        CustomThreshold    ///< 自定义阈值触发
    };
    Q_ENUM(PatternType)

    /** @brief 检测到的单个模式实例 */
    struct DetectedPattern {
        PatternType type = PatternType::Spike;  ///< 模式类型
        int startIndex = 0;                      ///< 模式起始索引(在分析缓冲区内)
        int endIndex = 0;                        ///< 模式结束索引
        double amplitude = 0.0;                  ///< 模式幅度(峰峰值)
        double periodMs = 0.0;                   ///< 周期(ms)，非周期模式为0
        double confidence = 0.0;                 ///< 置信度(0.0-1.0)
        QString description;                     ///< 人类可读描述
    };

    /** @brief 检测配置参数 */
    struct DetectionConfig {
        double spikeThreshold = 3.0;    ///< 毛刺判定阈值(标准差倍数)
        double dropoutThreshold = 0.1;  ///< 跌落判定阈值(均值比例)
        int minPatternLength = 5;       ///< 最小模式长度(采样点数)
        int analysisWindowSize = 256;   ///< 分析窗口大小(采样点数)
        bool detectPeriodic = true;     ///< 是否启用周期模式检测
        bool detectSpikes = true;       ///< 是否启用毛刺检测
        bool detectDropouts = true;     ///< 是否启用跌落检测
    };

    /** @brief 运行统计数据 */
    struct Stats {
        quint64 totalSamplesProcessed = 0;  ///< 累计处理采样点数
        quint64 totalPatternsDetected = 0;  ///< 累计检测到的模式总数
        quint64 totalPeriodicFound = 0;     ///< 累计检测到的周期模式数
        quint64 totalSpikesFound = 0;       ///< 累计检测到的毛刺数
        quint64 totalDropoutsFound = 0;     ///< 累计检测到的跌落数
        quint64 totalLevelChanges = 0;      ///< 累计检测到的电平跳变数
        quint64 totalAnalysisRuns = 0;      ///< 累计分析运行次数
        double peakConfidence = 0.0;        ///< 历史最高置信度
        double avgConfidence = 0.0;         ///< 平均置信度
    };

    /** @brief 构造波形模式检测器 @param parent 父对象 */
    explicit WaveformPatternDetector(QObject* parent = nullptr);

    /** @brief 设置检测配置参数 @param config 新配置 */
    void setConfig(const DetectionConfig& config);

    /** @brief 获取当前检测配置 @return 配置常量引用 */
    const DetectionConfig& config() const;

    /** @brief 喂入单个采样值，追加到环形缓冲区并更新在线统计 @param value 采样值 */
    void feedSample(double value);

    /** @brief 批量喂入采样值 @param samples 采样值向量 */
    void feedSamples(const QVector<double>& samples);

    /** @brief 执行分析 -- 运行所有已启用的检测算法，发射检测到的模式 */
    void analyze();

    /** @brief 获取所有已检测到的模式 @return 模式列表 */
    QList<DetectedPattern> detectedPatterns() const;

    /** @brief 按类型筛选已检测到的模式 @param type 目标模式类型 @return 匹配的模式列表 */
    QList<DetectedPattern> patternsByType(PatternType type) const;

    /** @brief 获取置信度最高的模式 @return 最佳模式(若无模式则type=Spike且confidence=0) */
    DetectedPattern bestPattern() const;

    /** @brief 估计信号主频率(Hz) -- 从最佳周期模式推导 @return 频率估计值，无周期模式时返回0 */
    double estimatedFrequency() const;

    /** @brief 获取运行统计数据 @return Stats常量引用 */
    const Stats& stats() const;

    /** @brief 重置所有统计计数器为初始值 */
    void resetStatistics();

    /** @brief 清空采样缓冲区并重置在线统计 */
    void clearBuffer();

signals:
    /** @brief 检测到新模式 @param pattern 检测到的模式描述 */
    void patternDetected(const DetectedPattern& pattern);

    /** @brief 一次分析完成 @param patternCount 本次检测到的模式数量 */
    void analysisComplete(int patternCount);

private:
    /** @brief 在线更新均值和标准差(Welford算法) @param value 新采样值 */
    void updateRunningStats(double value);

    /** @brief 毛刺检测 -- 检查缓冲区内超出spikeThreshold*stddev的异常点 */
    void detectSpikes();

    /** @brief 跌落检测 -- 检查缓冲区内低于dropoutThreshold*mean的区域 */
    void detectDropouts();

    /** @brief 周期模式检测 -- 使用自相关函数识别重复模式 */
    void detectPeriodicPatterns();

    /** @brief 电平跳变检测 -- 识别信号均值的突然偏移 */
    void detectLevelChanges();

    /** @brief 计算归一化自相关函数 @param data 输入数据序列 @param lag 滞后阶数 @return lag处的归一化自相关值(-1~1) */
    double autocorrelation(const QVector<double>& data, int lag) const;

    DetectionConfig m_config;           ///< 检测配置参数
    QVector<double> m_buffer;           ///< 环形采样缓冲区
    QList<DetectedPattern> m_patterns;  ///< 已检测到的模式列表
    int m_sampleIndex;                  ///< 全局采样索引(累计计数)

    Stats m_stats;                      ///< 运行统计数据

    double m_runningMean;               ///< 在线均值(Welford)
    double m_runningM2;                 ///< Welford M2辅助量(用于计算方差)
    quint64 m_runningCount;             ///< 在线统计采样计数

    static constexpr double kConfidenceScale = 100.0; ///< 置信度缩放因子
};

#endif // WAVEFORMPATTERNDETECTOR_H
