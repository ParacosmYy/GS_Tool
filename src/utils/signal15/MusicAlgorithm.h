/**
 * @file MusicAlgorithm.h
 * @brief MUSIC算法 — 多信号分类频率估计与波达方向(DOA)估计
 *
 * 功能: 基于协方差矩阵特征分解，利用信号子空间与噪声子空间的正交性，
 *       构造MUSIC伪谱进行高分辨率频率估计和DOA估计。
 *       支持均匀线阵(ULA)模型、前向/后向空间平滑。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / HilbertTransform(解析信号)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief MUSIC算法引擎 — 高分辨率频率估计与DOA估计
 */
class MusicAlgorithm : public QObject {
    Q_OBJECT

public:
    /** @brief MUSIC估计模式 */
    enum class Mode {
        FrequencyEstimation,    ///< 频率估计
        DOAEstimation           ///< 波达方向(DOA)估计
    };
    Q_ENUM(Mode)

    /** @brief 估计结果 */
    struct MusicResult {
        QVector<double> pseudoSpectrum;        ///< 伪谱值
        QVector<double> searchGrid;            ///< 搜索网格(频率/角度)
        QVector<double> estimatedValues;       ///< 估计的频率(Hz)或角度(deg)
        double resolution = 0.0;               ///< 估计分辨率
        int signalCount = 0;                   ///< 检测到的信号数
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        int totalEstimations = 0;              ///< 累计估计次数
        int totalSnapshotsProcessed = 0;       ///< 累计处理快照数
        int totalSignalsDetected = 0;          ///< 累计检测信号数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit MusicAlgorithm(QObject* parent = nullptr);

    /** @brief 设置估计模式 @param mode 模式 */
    void setMode(Mode mode);

    /** @brief 设置信号源数 @param count 信号数 */
    void setSignalCount(int count);

    /** @brief 设置阵列/快照参数 @param sensors 传感器数 @param snapshots 快照数 */
    void setArrayParams(int sensors, int snapshots);

    /** @brief 设置搜索网格密度 @param points 搜索点数 */
    void setSearchPoints(int points);

    /** @brief 设置采样率/波长参数 @param fs 采样率(频率模式)或波长(DOA模式) */
    void setSamplingRate(double fs);

    /** @brief 执行MUSIC估计 @param data 输入数据[snapshots x sensors] @return 估计结果 */
    MusicResult estimate(const QVector<QVector<double>>& data);

    /** @brief 自动检测信号数(基于MDL/AIC) @param data 输入数据 @return 检测到的信号数 */
    int detectSignalCount(const QVector<QVector<double>>& data);

    /** @brief 获取当前统计 @return 统计常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 估计完成 @param signals 信号数 @param timeMs 耗时 */
    void estimationComplete(int signals, double timeMs);

private:
    /** @brief 计算采样协方差矩阵 @param data 输入数据 @return 协方差矩阵 */
    QVector<QVector<double>> computeCovariance(
        const QVector<QVector<double>>& data) const;

    /** @brief 对称矩阵特征分解(Jacobi) @param mat 输入矩阵 @param eigenvalues 输出特征值 @param eigenvectors 输出特征向量 */
    void eigenDecompose(const QVector<QVector<double>>& mat,
                        QVector<double>& eigenvalues,
                        QVector<QVector<double>>& eigenvectors);

    /** @brief 构造MUSIC伪谱 @param noiseSubspace 噪声子空间 @return 伪谱值 */
    QVector<double> computePseudoSpectrum(
        const QVector<QVector<double>>& noiseSubspace);

    /** @brief 从伪谱峰值提取估计值 @param pseudoSpectrum 伪谱 @param grid 搜索网格 @return 估计值列表 */
    QVector<double> extractPeaks(
        const QVector<double>& pseudoSpectrum,
        const QVector<double>& grid);

    Mode m_mode;                        ///< 估计模式
    int m_signalCount;                  ///< 信号源数
    int m_sensors;                      ///< 传感器数
    int m_snapshots;                    ///< 快照数
    int m_searchPoints;                 ///< 搜索网格密度
    double m_samplingRate;              ///< 采样率

    Stats m_stats;                      ///< 运行时统计
    double m_timeSum = 0.0;             ///< 累计耗时(ms)
};
