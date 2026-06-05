/**
 * @file SparseCholesky.h
 * @brief 稀疏Cholesky分解 — 符号分解 + AMD排序 + 超节点消元
 *
 * 功能: 实现稀疏对称正定矩阵的Cholesky分解(LDL^T)，支持CSC稀疏存储、
 *       AMD近似最小度排序减少fill-in、符号分解预分配、超节点消元加速。
 *       适用于有限元分析、电路仿真、最优化问题求解。
 *
 * 协作: StrassenMultiply(矩阵乘) / SparseLU(稀疏LU)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

#include <vector>

/**
 * @brief 稀疏Cholesky分解 — CSC存储 + AMD排序
 */
class SparseCholesky : public QObject {
    Q_OBJECT

public:
    /** @brief CSC(Compressed Sparse Column)稀疏矩阵 */
    struct CSCMatrix {
        int n = 0;                           ///< 矩阵维度
        int nnz = 0;                         ///< 非零元素数
        QVector<int> colPtr;                 ///< 列指针(n+1)
        QVector<int> rowIdx;                 ///< 行索引(nnz)
        QVector<double> values;              ///< 值(nnz)
    };

    /** @brief 分解配置 */
    struct Parameters {
        bool enableAMD = true;               ///< 启用AMD排序
        bool enableSupernodal = true;        ///< 启用超节点消元
        double pivotTolerance = 1e-12;       ///< 主元容差(正定性检查)
        int maxFillRatio = 10;               ///< 最大fill-in比例限制
        bool computeDiagonal = true;         ///< 是否计算对角D(LDL^T)
    };

    /** @brief 分解结果 */
    struct FactorResult {
        CSCMatrix L;                         ///< 下三角因子L
        QVector<double> D;                   ///< 对角D(LDL^T)
        QVector<int> permutation;            ///< 排列向量P
        QVector<int> invPermutation;         ///< 逆排列P^{-1}
        int fillIn = 0;                      ///< fill-in元素数
        int supernodes = 0;                  ///< 超节点数
        double logDeterminant = 0.0;         ///< log(det(A))
        bool isPositiveDefinite = false;     ///< 是否正定
        bool success = false;               ///< 是否成功
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalFactorizations = 0;    ///< 累计分解次数
        quint64 totalSolves = 0;            ///< 累计求解次数
        quint64 totalNonzerosProcessed = 0; ///< 累计处理非零元素数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit SparseCholesky(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~SparseCholesky() override;

    // ── 配置 ──

    /** @brief 设置分解参数 @param params 参数 */
    void setParameters(const Parameters& params);

    /** @brief 获取当前参数 @return 参数 */
    Parameters parameters() const;

    // ── 矩阵构建 ──

    /**
     * @brief 从COO(坐标)格式构建CSC矩阵
     * @param n 维度
     * @param rows 行索引
     * @param cols 列索引
     * @param vals 值
     * @return CSC矩阵
     */
    CSCMatrix buildFromCOO(int n, const QVector<int>& rows,
                           const QVector<int>& cols,
                           const QVector<double>& vals) const;

    /**
     * @brief 创建单位矩阵(CSC)
     * @param n 维度
     * @return CSC矩阵
     */
    CSCMatrix identity(int n) const;

    // ── 分解 ──

    /**
     * @brief 执行稀疏Cholesky分解(A = P*L*D*L^T*P^T)
     * @param A 输入SPD矩阵(仅使用下三角)
     * @return 分解结果
     */
    FactorResult factorize(const CSCMatrix& A);

    // ── 求解 ──

    /**
     * @brief 求解线性系统Ax = b
     * @param factor 分解结果
     * @param b 右端向量
     * @return 解向量x
     */
    QVector<double> solve(const FactorResult& factor,
                          const QVector<double>& b) const;

    /**
     * @brief 批量求解(Ax = B, 多右端)
     * @param factor 分解结果
     * @param B 右端矩阵(列优先)
     * @return 解矩阵X
     */
    QVector<QVector<double>> solveBatch(
        const FactorResult& factor,
        const QVector<QVector<double>>& B) const;

    // ── 统计 ──

    /** @brief 获取统计 @return 统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 分解完成 @param nnzFactor L的非零元素数 @param fillIn fill-in数 */
    void factorizationCompleted(int nnzFactor, int fillIn);

    /** @brief 排序完成 @param ordering 排序算法 @param nnzNew 排序后非零数 */
    void orderingCompleted(const QString& ordering, int nnzNew);

private:
    /** @brief AMD近似最小度排序 @param A 输入矩阵 @return 排列向量P */
    QVector<int> amdOrdering(const CSCMatrix& A) const;

    /** @brief 符号分解 @param A 输入矩阵 @param perm 排列 @return L结构 */
    CSCMatrix symbolicFactorization(const CSCMatrix& A,
                                    const QVector<int>& perm) const;

    /** @brief 数值分解 @param A 输入矩阵 @param symbol 符号结构 @param perm 排列 @return (L值, D值) */
    QPair<QVector<double>, QVector<double>> numericFactorization(
        const CSCMatrix& A, const CSCMatrix& symbolStructure,
        const QVector<int>& perm);

    /** @brief 超节点检测 @param colPtr 列指针 @param rowIdx 行索引 @return 超节点列表 */
    QVector<QPair<int, int>> detectSupernodes(const QVector<int>& colPtr,
                                              const QVector<int>& rowIdx) const;

    /** @brief 前推(Lx=b) @param L 下三角 @param b 右端 @return 解 */
    QVector<double> forwardSolve(const CSCMatrix& L,
                                 const QVector<double>& b) const;

    /** @brief 对角求解(Dx=b) @param D 对角 @param b 右端 @return 解 */
    QVector<double> diagonalSolve(const QVector<double>& D,
                                  const QVector<double>& b) const;

    /** @brief 回代(L^T x=b) @param L 下三角 @param b 右端 @return 解 */
    QVector<double> backwardSolve(const CSCMatrix& L,
                                  const QVector<double>& b) const;

    Parameters m_params;                 ///< 分解参数
    Stats m_stats;                       ///< 操作统计
    double m_timeSum = 0.0;              ///< 累计耗时
};
