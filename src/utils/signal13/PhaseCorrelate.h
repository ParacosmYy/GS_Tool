/**
 * @file PhaseCorrelate.h
 * @brief 相位相关算法 — 图像/信号配准的频域平移估计
 *
 * 功能: 实现基于FFT的相位相关方法，估计两幅图像或两段信号之间
 *       的亚像素级平移偏移。支持2D图像配准、1D信号对齐、
 *       互功率谱计算。适用于图像拼接、运动估计、信号同步。
 *
 * 协作: AdaptiveFFT(FFT计算) / DataCorrelator(相关分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QImage>

/**
 * @brief 相位相关配准引擎 — 频域平移估计
 */
class PhaseCorrelate : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalCorrelations = 0;             ///< 累计相位相关次数
        int totalFFTsPerformed = 0;            ///< 累计FFT执行次数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    /** @brief 亚像素位移结果 */
    struct ShiftResult {
        double dx = 0.0;       ///< 水平位移(像素/样本)
        double dy = 0.0;       ///< 垂直位移(像素, 2D时有效)
        double confidence = 0.0;///< 峰值置信度[0,1]
        double peakValue = 0.0; ///< 归一化互相关峰值
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PhaseCorrelate(QObject* parent = nullptr);

    /**
     * @brief 1D信号相位相关
     * @param signal1 参考信号
     * @param signal2 待对齐信号
     * @return 位移结果(dx有效, dy=0)
     */
    ShiftResult correlate1D(const QVector<double>& signal1,
                            const QVector<double>& signal2);

    /**
     * @brief 2D图像相位相关
     * @param image1 参考图像
     * @param image2 待对齐图像
     * @return 位移结果(dx, dy均有效)
     */
    ShiftResult correlate2D(const QImage& image1, const QImage& image2);

    /**
     * @brief 2D矩阵相位相关(灰度值)
     * @param mat1 参考矩阵(行优先)
     * @param mat2 待对齐矩阵
     * @param rows 行数
     * @param cols 列数
     * @return 位移结果
     */
    ShiftResult correlate2DMatrix(const QVector<double>& mat1,
                                  const QVector<double>& mat2,
                                  int rows, int cols);

    /**
     * @brief 计算互功率谱
     * @param fft1 信号1的FFT
     * @param fft2 信号2的FFT
     * @return 归一化互功率谱(复数: 实部/虚部交织)
     */
    QVector<double> crossPowerSpectrum(const QVector<double>& fft1,
                                       const QVector<double>& fft2) const;

    /**
     * @brief 亚像素精度峰值检测(质心法)
     * @param data 相关输出
     * @param size 数据长度
     * @param peakIdx 整数峰值位置
     * @return 亚像素偏移量
     */
    double subPixelPeak1D(const QVector<double>& data, int peakIdx) const;

    /**
     * @brief 获取统计信息
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief 配准完成
     * @param dx 水平位移
     * @param dy 垂直位移
     * @param confidence 置信度
     */
    void correlationCompleted(double dx, double dy, double confidence);

private:
    /**
     * @brief 1D FFT(Cooley-Tukey, 就地)
     * @param re 实部数组
     * @param im 虚部数组
     * @param inverse 是否逆变换
     */
    void fft1D(QVector<double>& re, QVector<double>& im, bool inverse) const;

    /**
     * @brief 2D FFT(行列分解)
     * @param re 实部矩阵(行优先)
     * @param im 虚部矩阵
     * @param rows 行数
     * @param cols 列数
     * @param inverse 是否逆变换
     */
    void fft2D(QVector<double>& re, QVector<double>& im,
               int rows, int cols, bool inverse) const;

    /**
     * @brief 补零到2的幂次
     */
    static int nextPowerOfTwo(int n);

    Stats m_stats;
    double m_timeSum = 0.0;                     ///< 处理时间累加器
};
