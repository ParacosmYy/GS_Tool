/**
 * @file ArnoldiProcess.h
 * @brief Arnoldi迭代 — 非对称矩阵的Krylov子空间正交化
 *
 * 功能: 对非对称矩阵A构建Arnoldi分解 A*Q = Q*H + h_{m+1,m}*q_{m+1}*e_m^T，
 *       其中Q为正交基，H为上Hessenberg矩阵。用于GMRES、特征值近似等。
 *       统计构建次数和平均耗时。
 */
#ifndef ARNOLDIPROCESS_H
#define ARNOLDIPROCESS_H

#include <QObject>
#include <QPair>
#include <QVector>

#include <functional>

/**
 * @class ArnoldiProcess
 * @brief Arnoldi迭代工具类，构建Krylov子空间正交基
 */
class ArnoldiProcess : public QObject {
    Q_OBJECT
public:
    /** 统计信息结构体 */
    struct Stats {
        quint64 totalBuilds = 0;        /**< 总构建次数 */
        double  avgProcessingTimeMs = 0.0; /**< 平均处理耗时(ms) */
    };

    /**
     * @brief 构造函数
     * @param parent 父QObject
     */
    explicit ArnoldiProcess(QObject* parent = nullptr);

    /**
     * @brief 构建Arnoldi分解
     * @param matvec 矩阵-向量乘法函数对象
     * @param n 向量维度
     * @param numVectors Krylov子空间维度(迭代次数)
     * @param maxIter 最大迭代次数(0表示使用numVectors)
     * @return QPair{Q, H}，Q为n x k正交基，H为(k+1) x k上Hessenberg矩阵
     */
    QPair<QVector<QVector<double>>, QVector<QVector<double>>>
    build(std::function<QVector<double>(const QVector<double>&)> matvec,
          int n, int numVectors, int maxIter = 0);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 构建完成信号 @param numVectors 实际构建的向量数 */
    void buildCompleted(int numVectors);

private:
    Stats  m_stats;   /**< 统计数据 */
    double m_timeSum; /**< 累计耗时(ms) */
};

#endif // ARNOLDIPROCESS_H
