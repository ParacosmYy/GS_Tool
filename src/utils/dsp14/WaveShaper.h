/**
 * @file WaveShaper.h
 * @brief 波形塑形/失真处理器 — 传输函数查找表 + 过采样 + 抗混叠
 *
 * 功能: 通过可配置的传输函数曲线对信号进行非线性波形塑形。
 *       内置过采样(2x/4x/8x)和低通抗混叠滤波，防止谐波折叠。
 *       支持多种预设失真曲线(tanh/soft-clip/hard-clip/fuzz/custom)。
 *       统计处理次数/峰值样本数/平均处理耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class WaveShaper
 * @brief 非线性波形塑形处理器
 */
class WaveShaper : public QObject {
    Q_OBJECT
public:
    /** 失真类型 */
    enum DistortionType {
        TanhSoft = 0,    ///< 双曲正切软削波
        SoftClip,        ///< 三次多项式软削波
        HardClip,        ///< 硬削波
        Fuzz,            ///< 模糊失真(全波整流+削波)
        TapeSaturation,  ///< 磁带饱和(指数压缩)
        Custom           ///< 自定义查找表
    };
    Q_ENUM(DistortionType)

    /** 过采样倍率 */
    enum OversampleFactor {
        None = 1,   ///< 无过采样
        X2 = 2,     ///< 2倍过采样
        X4 = 4,     ///< 4倍过采样
        X8 = 8      ///< 8倍过采样
    };
    Q_ENUM(OversampleFactor)

    /** 处理统计 */
    struct Stats {
        quint64 totalBlocksProcessed = 0;  ///< 总处理块数
        quint64 totalSamplesProcessed = 0; ///< 累计处理样本数
        quint64 totalPeakClips = 0;        ///< 累计削波次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** 构造函数 */
    explicit WaveShaper(QObject* parent = nullptr);

    /** @brief 设置失真类型 @param type 预设类型 */
    void setDistortionType(DistortionType type);

    /** @brief 设置驱动增益 @param gain 输入增益(0.1~10.0) */
    void setDrive(double gain);

    /** @brief 设置输出混合比 @param mix 干/湿信号混合(0=全干, 1=全湿) */
    void setMix(double mix);

    /** @brief 设置过采样倍率 @param factor 过采样因子 */
    void setOversample(OversampleFactor factor);

    /** @brief 设置自定义传输函数 @param table 查找表(256项, x映射到[-1,1]) */
    void setCustomTable(const QVector<double>& table);

    /**
     * @brief 处理一个音频块
     * @param input 输入样本(范围[-1,1])
     * @return 处理后的样本
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 处理单个样本
     * @param sample 输入样本
     * @return 处理后样本
     */
    double processOne(double sample);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 处理完成 @param samples 样本数 @param peaks 削波数 */
    void blockProcessed(int samples, int peaks);

private:
    /** 重建预设查找表 */
    void rebuildTable();
    /** 传输函数查找(线性插值) */
    double lookupTransfer(double x) const;
    /** 过采样(线性插值上采样) */
    QVector<double> upsample(const QVector<double>& input) const;
    /** 降采样(均值滤波) */
    QVector<double> downsample(const QVector<double>& input) const;
    /** 低通抗混叠滤波器 */
    void applyAntiAliasFilter(QVector<double>& data) const;

    static constexpr int TABLE_SIZE = 256; ///< 查找表大小

    DistortionType m_type;            ///< 失真类型
    double m_drive;                   ///< 驱动增益
    double m_mix;                     ///< 干/湿混合
    OversampleFactor m_oversample;    ///< 过采样倍率
    QVector<double> m_transferTable;  ///< 传输函数查找表

    Stats  m_stats;         ///< 统计信息
    double m_timeSum = 0.0; ///< 累计耗时
};
