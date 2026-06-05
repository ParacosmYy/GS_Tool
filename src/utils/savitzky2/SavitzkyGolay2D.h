/**
 * @file SavitzkyGolay2D.h
 * @brief 二维Savitzky-Golay滤波器 — 图像/矩阵平滑与微分
 *
 * 功能: 基于局部二维多项式最小二乘拟合的二维平滑滤波器。
 *       支持任意矩形窗口、多项式阶数和偏导阶数。
 *       适用于图像去噪、二维信号平滑和梯度计算等。
 *
 * 协作: SavitzkyGolay(一维S-G滤波) / DigitalFilter(一般滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 二维Savitzky-Golay滤波器 — 图像/矩阵平滑微分
 */
class SavitzkyGolay2D : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalFiltered = 0;         ///< 累计滤波次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造二维S-G滤波器
     * @param rows 输入矩阵行数(仅用于默认参数)
     * @param cols 输入矩阵列数(仅用于默认参数)
     * @param polyOrder 多项式阶数(默认2)
     * @param derivOrder 偏导阶数(0=平滑, 1=一阶偏导, 2=二阶偏导)
     * @param parent 父对象
     */
    explicit SavitzkyGolay2D(int rows = 0, int cols = 0,
                             int polyOrder = 2, int derivOrder = 0,
                             QObject* parent = nullptr);

    /**
     * @brief 对二维数据执行S-G滤波
     * @param input 输入矩阵(外层为行, 内层为列)
     * @return 滤波后矩阵
     *
     * 输入必须为非空的规整二维数组(每行长度相同)。
     * 边界采用镜像扩展处理。
     */
    QVector<QVector<double>> filter(const QVector<QVector<double>>& input);

    /**
     * @brief 设置卷积核窗口大小
     * @param rows 窗口行数(必须为奇数, >= 3)
     * @param cols 窗口列数(必须为奇数, >= 3)
     *
     * 窗口越大平滑越强但细节越少; 奇数参数会自动修正为偶数+1。
     */
    void setKernelSize(int rows, int cols);

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 滤波完成 @param rows 输出行数 @param cols 输出列数 */
    void filterCompleted(int rows, int cols);

private:
    /**
     * @brief 计算二维S-G卷积核
     * @return 展平后的卷积核系数(行优先)
     *
     * 构建二维Vandermonde矩阵并通过正规方程求解最小二乘。
     */
    QVector<double> computeKernel() const;

    /**
     * @brief 生成二维单项式基向量
     * @param x x坐标偏移
     * @param y y坐标偏移
     * @return 单项式值向量(按幂次排列: 1,x,y,x^2,xy,y^2,...)
     */
    QVector<double> basisVector(double x, double y) const;

    int m_kernelRows;               ///< 卷积核行数
    int m_kernelCols;               ///< 卷积核列数
    int m_polyOrder;                ///< 多项式阶数
    int m_derivOrder;               ///< 偏导阶数
    QVector<double> m_kernel;       ///< 预计算的卷积核系数

    Stats  m_stats;                 ///< 统计信息
    double m_timeSum;               ///< 处理时间累加器
};
