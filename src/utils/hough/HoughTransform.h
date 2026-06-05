/**
 * @file HoughTransform.h
 * @brief 霍夫变换(Hough Transform) — 直线检测
 *
 * 功能: 实现标准霍夫变换，从二值图像中检测直线。
 *       支持角度/距离分辨率配置和投票阈值设置。
 *       统计检测次数/线段数/平均耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class HoughTransform
 * @brief 霍夫变换直线检测器
 *
 * 将图像空间中的边缘点映射到参数空间(θ,ρ)，
 * 通过累加器投票检测直线。参数空间按指定分辨率离散化。
 */
class HoughTransform : public QObject
{
    Q_OBJECT

public:
    /** @brief 检测到的直线结果 */
    struct Line {
        double theta = 0.0;  ///< 直线角度(弧度)
        double rho = 0.0;    ///< 原点到直线距离(像素)
        int    votes = 0;    ///< 累加器票数
    };

    /** @brief 运行统计信息 */
    struct Stats {
        quint64 totalDetected = 0;   ///< 总检测次数
        quint64 totalLines = 0;      ///< 累计检测到的线段数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit HoughTransform(QObject* parent = nullptr);

    /**
     * @brief 检测二值图像中的直线
     * @param image 二值图像(非零视为边缘点)，image[row][col]
     * @param threshold 最小投票阈值
     * @return 检测到的直线列表(按票数降序)
     */
    QVector<Line> detect(const QVector<QVector<int>>& image, int threshold);

    /**
     * @brief 设置参数空间分辨率
     * @param thetaRes 角度分辨率(弧度，默认π/180)
     * @param rhoRes 距离分辨率(像素，默认1.0)
     */
    void setResolution(double thetaRes, double rhoRes);

    /** @brief 获取统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 检测完成信号 @param lineCount 检测到的线段数 */
    void detectionCompleted(int lineCount);

private:
    /**
     * @brief 在累加器中投票
     * @param row 边缘点行坐标
     * @param col 边缘点列坐标
     * @param accumulator 累加器矩阵
     * @param thetaBins 角度分箱数
     * @param maxRho 最大距离
     */
    void vote(int row, int col,
              QVector<QVector<int>>& accumulator,
              int thetaBins, double maxRho) const;

    /**
     * @brief 从累加器提取峰值
     * @param accumulator 累加器矩阵
     * @param threshold 最小票数阈值
     * @return 检测到的直线列表
     */
    QVector<Line> extractPeaks(
        const QVector<QVector<int>>& accumulator,
        int threshold) const;

    mutable Stats m_stats;          ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
    double m_thetaRes = M_PI / 180.0; ///< 角度分辨率
    double m_rhoRes = 1.0;          ///< 距离分辨率
};
