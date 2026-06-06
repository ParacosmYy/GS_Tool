/**
 * @file Beamformer2.h
 * @brief MVDR(Capon)波束形成器 — MVDR (Capon) Beamformer with Sample Covariance Matrix Inversion
 *
 * 功能: 实现MVDR(Capon)自适应波束形成器，基于采样协方差矩阵求逆。
 *       支持多阵元阵列、任意导向矢量和正则化对角加载。
 *       提供阵列输出、空间谱和最优权值计算。
 *
 * 协作: FftEngine(FFT) / WindowEngine(窗函数) / Correlator(相关器)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief MVDR波束形成器
 */
class Beamformer2 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalBeamforms = 0;            ///< 累计波束形成次数
        quint64 totalSpectrumCalcs = 0;        ///< 累计谱计算次数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理耗时(ms)
        double lastSnrImprovementDb = 0.0;     ///< 最近SNR改善(dB)
    };

    explicit Beamformer2(QObject* parent = nullptr);
    ~Beamformer2() override;

    /** @brief 设置阵元数 */
    void setNumSensors(int n);
    /** @brief 设置正则化对角加载因子 */
    void setDiagonalLoading(double loading);
    /** @brief 设置采样快拍数 */
    void setSnapshotCount(int count);

    /**
     * @brief 计算MVDR最优权值
     * @param snapshots 快拍数据 [snapshot][sensor]
     * @param steeringVector 期望信号导向矢量(长度=numSensors)
     * @return 最优权值
     */
    QVector<double> computeWeights(const QVector<QVector<double>>& snapshots,
                                   const QVector<double>& steeringVector);

    /**
     * @brief 应用波束形成(单快拍)
     * @param weights 权值
     * @param snapshot 单快拍数据
     * @return 波束形成输出
     */
    double applyBeamform(const QVector<double>& weights,
                         const QVector<double>& snapshot) const;

    /**
     * @brief 计算MVDR空间功率谱
     * @param snapshots 快拍数据
     * @param steeringVectors 扫描导向矢量列表
     * @return 各方向功率值
     */
    QVector<double> spatialSpectrum(const QVector<QVector<double>>& snapshots,
                                    const QVector<QVector<double>>& steeringVectors);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 波束形成完成 @param numSensors 阵元数 */
    void beamformCompleted(int numSensors);
    /** @brief 空间谱计算完成 @param numDirections 方向数 */
    void spectrumCompleted(int numDirections);

private:
    /** @brief 计算采样协方差矩阵 */
    void sampleCovariance(const QVector<QVector<double>>& snapshots,
                          QVector<QVector<double>>& R) const;

    /** @brief 矩阵求逆(Gauss-Jordan) */
    bool invertMatrix(QVector<QVector<double>>& mat) const;

    /** @brief Hermitian内积(实数版本) */
    static double innerProduct(const QVector<double>& a, const QVector<double>& b);

    /** @brief 矩阵向量乘 */
    static QVector<double> matVecMultiply(const QVector<QVector<double>>& M,
                                          const QVector<double>& v);

    int m_numSensors = 8;
    double m_diagonalLoading = 1e-4;
    int m_snapshotCount = 100;

    Stats m_stats;
    double m_timeSum = 0.0;
};
