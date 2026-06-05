/**
 * @file Convolution2D.h
 * @brief 二维卷积 — 多种边界处理模式/可分离卷积
 *
 * 功能: 对二维矩阵执行卷积运算，支持reflect/zero/replicate/wrap
 *       四种边界模式，提供可分离卷积以加速大核计算。
 *
 * 协作: MorphologicalOps(形态学) / DigitalFilter(滤波器)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 二维卷积运算器
 */
class Convolution2D : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalConvolutions = 0;     ///< 累计卷积次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit Convolution2D(QObject* parent = nullptr);

    /**
     * @brief 二维卷积
     * @param input 输入矩阵
     * @param kernel 卷积核
     * @param borderMode 边界模式: "reflect"/"zero"/"replicate"/"wrap"
     * @return 卷积结果
     */
    QVector<QVector<double>> convolve(
        const QVector<QVector<double>>& input,
        const QVector<QVector<double>>& kernel,
        const QString& borderMode = "reflect");

    /**
     * @brief 设置默认边界模式
     * @param mode 边界模式
     */
    void setBorderMode(const QString& mode);

    /**
     * @brief 可分离卷积(先X后Y方向)
     * @param input 输入矩阵
     * @param kernelX X方向1D核
     * @param kernelY Y方向1D核
     * @return 卷积结果
     */
    QVector<QVector<double>> separableConvolve(
        const QVector<QVector<double>>& input,
        const QVector<double>& kernelX,
        const QVector<double>& kernelY);

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 卷积完成 @param rows 结果行数 @param cols 结果列数 */
    void convolutionCompleted(int rows, int cols);

private:
    /**
     * @brief 边界扩展取值
     * @param input 输入矩阵
     * @param row 行索引(可越界)
     * @param col 列索引(可越界)
     * @param mode 边界模式
     * @return 扩展后的值
     */
    double borderValue(const QVector<QVector<double>>& input,
                       int row, int col, const QString& mode) const;

    Stats m_stats;           ///< 统计信息
    double m_timeSum;        ///< 累计耗时
    QString m_borderMode;    ///< 默认边界模式
};
