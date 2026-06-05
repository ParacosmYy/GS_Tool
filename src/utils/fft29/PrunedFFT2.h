/**
 * @file PrunedFFT2.h
 * @brief 剪枝FFT增强 — 输入/输出剪枝/稀疏频谱/自适应剪枝
 *
 * 针对稀疏信号优化的FFT实现:
 *   - 输入剪枝: 跳过零值输入的蝶形运算
 *   - 输出剪枝: 只计算需要的频率分量
 *   - 稀疏频谱检测: 自动识别需要计算的频段
 *   - 自适应剪枝: 运行时根据稀疏度选择最优策略
 * 统计剪枝节省的运算量和处理时间。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include <QSet>

/**
 * @brief 剪枝FFT处理器
 */
class PrunedFFT2 : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalTransforms = 0;         ///< 总变换次数
        quint64 totalButterfliesSkipped = 0; ///< 总跳过的蝶形运算数
        quint64 totalButterfliesTotal = 0;   ///< 总蝶形运算数(含跳过)
        double avgProcessingTimeMs = 0.0;    ///< 平均处理耗时(ms)
        double avgSparsityRatio = 0.0;       ///< 平均稀疏度
    };

    /** 剪枝模式 */
    enum class PruneMode {
        InputPrune,    ///< 输入剪枝(输入稀疏时高效)
        OutputPrune,   ///< 输出剪枝(只需部分频谱时高效)
        Adaptive       ///< 自适应(根据稀疏度自动选择)
    };
    Q_ENUM(PruneMode)

    /**
     * @brief 构造函数
     * @param fftSize FFT长度(必须为2的幂)
     * @param mode 剪枝模式
     * @param parent 父对象
     */
    explicit PrunedFFT2(int fftSize = 1024, PruneMode mode = PruneMode::Adaptive,
                        QObject* parent = nullptr);

    /**
     * @brief 执行剪枝FFT(输入稀疏)
     * @param input 复数输入(交错实/虚)
     * @param nonzeroIndices 非零输入索引
     * @return 复数输出(交错实/虚)
     */
    QVector<double> transformInputPrune(const QVector<double>& input,
                                        const QVector<int>& nonzeroIndices);

    /**
     * @brief 执行剪枝FFT(输出选择)
     * @param input 复数输入(交错实/虚)
     * @param outputIndices 需要的输出频率索引
     * @return 选定频率的复数输出(交错实/虚)
     */
    QVector<double> transformOutputPrune(const QVector<double>& input,
                                         const QVector<int>& outputIndices);

    /**
     * @brief 自适应剪枝FFT
     * @param input 复数输入(交错实/虚)
     * @param sparsityHint 稀疏度提示(0.0=密集, 1.0=全零)
     * @return 全部频率的复数输出
     */
    QVector<double> transformAdaptive(const QVector<double>& input, double sparsityHint = -1.0);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief FFT长度 */
    int fftSize() const { return m_fftSize; }
    /** @brief 设置FFT长度 */
    void setFftSize(int size);
    /** @brief 上次跳过的蝶形运算数 */
    quint64 lastSkippedButterflies() const { return m_lastSkipped; }

signals:
    /** 变换完成 */
    void transformComplete(int fftSize, double sparsityRatio, double processingTimeMs);

private:
    /** 位反转置换 */
    void bitReverse(QVector<double>& data) const;
    /** 标准FFT基2蝶形 */
    void butterflyFull(QVector<double>& data);
    /** 输入剪枝蝶形 */
    void butterflyInputPrune(QVector<double>& data, const QSet<int>& nonzero);
    /** 输出剪枝蝶形 */
    void butterflyOutputPrune(const QVector<double>& input, QVector<double>& output,
                              const QSet<int>& outIndices);
    /** 计算当前输入的稀疏度 */
    double computeSparsity(const QVector<double>& input) const;
    /** 预计算旋转因子 */
    void computeTwiddles();

    int m_fftSize;
    int m_logSize;
    PruneMode m_mode;
    QVector<double> m_twiddles;
    quint64 m_lastSkipped;
    Stats m_stats;
    double m_timeSum = 0.0;
    QElapsedTimer m_timing;
};
