/**
 * @file MorphologicalOps.h
 * @brief 二值形态学运算 — 腐蚀/膨胀/开运算/闭运算/梯度
 *
 * 功能: 对二维二值图像执行经典形态学操作，支持自定义结构元素，
 *       提供开闭运算和形态学梯度等组合操作，用于图像预处理与特征提取。
 *
 * 协作: Convolution2D(邻域运算) / EdgeDetector(边缘提取)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 二值形态学运算器
 */
class MorphologicalOps : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalOperations = 0;     ///< 累计操作次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit MorphologicalOps(QObject* parent = nullptr);

    /**
     * @brief 腐蚀运算
     * @param binary 二值输入矩阵(0/1)
     * @param kernel 结构元素(0/1矩阵)
     * @return 腐蚀结果
     */
    QVector<QVector<int>> erode(const QVector<QVector<int>>& binary,
                                const QVector<QVector<int>>& kernel);

    /**
     * @brief 膨胀运算
     * @param binary 二值输入矩阵(0/1)
     * @param kernel 结构元素(0/1矩阵)
     * @return 膨胀结果
     */
    QVector<QVector<int>> dilate(const QVector<QVector<int>>& binary,
                                 const QVector<QVector<int>>& kernel);

    /**
     * @brief 开运算(先腐蚀后膨胀)
     * @param binary 二值输入矩阵
     * @param kernel 结构元素
     * @return 开运算结果
     */
    QVector<QVector<int>> open(const QVector<QVector<int>>& binary,
                               const QVector<QVector<int>>& kernel);

    /**
     * @brief 闭运算(先膨胀后腐蚀)
     * @param binary 二值输入矩阵
     * @param kernel 结构元素
     * @return 闭运算结果
     */
    QVector<QVector<int>> close(const QVector<QVector<int>>& binary,
                                const QVector<QVector<int>>& kernel);

    /**
     * @brief 形态学梯度(膨胀 - 腐蚀)
     * @param binary 二值输入矩阵
     * @param kernel 结构元素
     * @return 梯度结果
     */
    QVector<QVector<int>> gradient(const QVector<QVector<int>>& binary,
                                   const QVector<QVector<int>>& kernel);

    /**
     * @brief 设置默认结构元素
     * @param k 结构元素矩阵
     */
    void setKernel(const QVector<QVector<int>>& k);

    /** @brief 获取统计 @return 统计信息常引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 操作完成 @param opType 操作类型 @param rows 结果行数 @param cols 结果列数 */
    void operationCompleted(const QString& opType, int rows, int cols);

private:
    /**
     * @brief 内部腐蚀实现
     * @param input 输入矩阵
     * @param kernel 结构元素
     * @return 腐蚀结果
     */
    QVector<QVector<int>> erodeImpl(const QVector<QVector<int>>& input,
                                    const QVector<QVector<int>>& kernel) const;

    /**
     * @brief 内部膨胀实现
     * @param input 输入矩阵
     * @param kernel 结构元素
     * @return 膨胀结果
     */
    QVector<QVector<int>> dilateImpl(const QVector<QVector<int>>& input,
                                     const QVector<QVector<int>>& kernel) const;

    Stats m_stats;                          ///< 统计信息
    double m_timeSum;                       ///< 累计耗时
    QVector<QVector<int>> m_defaultKernel;  ///< 默认结构元素
};
