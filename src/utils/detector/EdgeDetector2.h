/**
 * @file EdgeDetector2.h
 * @brief 边缘检测器 — Canny/Sobel/Prewitt多算法支持
 *
 * 提供3种经典图像边缘检测算法: Sobel算子、Prewitt算子、Canny完整流程。
 * 输入为灰度图像(一维数组), 输出为边缘强度图或二值边缘图。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class EdgeDetector2
 * @brief 边缘检测器 — Canny/Sobel/Prewitt
 *
 * 输入图像为灰度值一维数组(width * height), 值域[0,255]。
 * Sobel/Prewitt输出梯度幅值图; Canny输出二值边缘图(含非极大值抑制+双阈值)。
 */
class EdgeDetector2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 检测算法 */
    enum class Algorithm {
        Sobel,   ///< Sobel算子(3x3)
        Prewitt, ///< Prewitt算子(3x3)
        Canny    ///< Canny边缘检测(完整流程)
    };

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalDetections = 0;    ///< 总检测次数
        quint64 totalPixelsProcessed = 0; ///< 总处理像素数
        double  avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    /** @brief Canny参数 */
    struct CannyParams {
        double lowThreshold;     ///< 低阈值
        double highThreshold;   ///< 高阈值
        int gaussianKernelSize;     ///< 高斯核大小(奇数)
        double gaussianSigma;     ///< 高斯标准差

        CannyParams()
            : lowThreshold(50.0)
            , highThreshold(150.0)
            , gaussianKernelSize(5)
            , gaussianSigma(1.4)
        {}
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit EdgeDetector2(QObject* parent = nullptr);

    /**
     * @brief Sobel边缘检测
     * @param image 灰度图像 [width * height]
     * @param width 图像宽度
     * @param height 图像高度
     * @return 梯度幅值图 [width * height]
     */
    QVector<double> sobel(const QVector<double>& image,
                          int width, int height) const;

    /**
     * @brief Prewitt边缘检测
     * @param image 灰度图像
     * @param width 宽度
     * @param height 高度
     * @return 梯度幅值图
     */
    QVector<double> prewitt(const QVector<double>& image,
                            int width, int height) const;

    /**
     * @brief Canny边缘检测(完整流程)
     * @param image 灰度图像
     * @param width 宽度
     * @param height 高度
     * @param params Canny参数
     * @return 二值边缘图(0或255)
     */
    QVector<double> canny(const QVector<double>& image,
                          int width, int height,
                          const CannyParams& params) const;

    /**
     * @brief 高斯模糊
     * @param image 输入图像
     * @param width 宽度
     * @param height 高度
     * @param kernelSize 核大小(奇数)
     * @param sigma 标准差
     * @return 模糊后图像
     */
    QVector<double> gaussianBlur(const QVector<double>& image,
                                 int width, int height,
                                 int kernelSize, double sigma) const;

    /**
     * @brief 使用3x3卷积核滤波
     * @param image 输入图像
     * @param width 宽度
     * @param height 高度
     * @param kernel 3x3卷积核
     * @return 滤波结果
     */
    QVector<double> convolve3x3(const QVector<double>& image,
                                int width, int height,
                                const double kernel[9]) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 检测完成 @param algorithm 算法名 @param pixels 像素数 */
    void detectionCompleted(const QString& algorithm, int pixels);

private:
    /** @brief 生成高斯核 */
    QVector<double> createGaussianKernel(int size, double sigma) const;

    /** @brief 非极大值抑制(Canny步骤) */
    QVector<double> nonMaxSuppression(const QVector<double>& magnitude,
                                      const QVector<double>& direction,
                                      int width, int height) const;

    /** @brief 双阈值+滞后连接(Canny步骤) */
    QVector<double> hysteresisThreshold(const QVector<double>& suppressed,
                                        int width, int height,
                                        double lowThresh, double highThresh) const;

    mutable Stats m_stats;     ///< 操作统计
    mutable double m_timeSum = 0.0; ///< 累计耗时
};
