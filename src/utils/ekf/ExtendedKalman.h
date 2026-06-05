/**
 * @file ExtendedKalman.h
 * @brief 扩展卡尔曼滤波器 — 非线性系统Jacobian
 *
 * 功能: 通过对非线性状态转移和观测函数进行一阶Taylor展开，
 *       利用Jacobian矩阵将非线性系统线性化，实现扩展卡尔曼滤波。
 *       支持自定义状态/观测函数及对应的Jacobian。
 *
 * 协作: ParticleFilter(非高斯) / KalmanFilter1D(一维线性)
 */
#ifndef EXTENDEDKALMAN_H
#define EXTENDEDKALMAN_H

#include <QObject>
#include <QVector>
#include <functional>

/**
 * @brief 扩展卡尔曼滤波器 — 非线性系统Jacobian
 */
class ExtendedKalman : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalPredictions = 0;      ///< 累计预测次数
        quint64 totalUpdates = 0;          ///< 累计更新次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /** @brief 矩阵类型(QVector<QVector<double>>) */
    using Matrix = QVector<QVector<double>>;

    /** @brief 向量函数类型: (state) -> vector */
    using VecFunc = std::function<QVector<double>(const QVector<double>&)>;

    /** @brief Jacobian函数类型: (state) -> matrix */
    using JacobianFunc = std::function<Matrix(const QVector<double>&)>;

    explicit ExtendedKalman(QObject* parent = nullptr);

    /** @brief 初始化EKF
     *  @param state 初始状态
     *  @param covariance 初始协方差矩阵 */
    void initialize(const QVector<double>& state,
                    const Matrix& covariance);

    /** @brief 预测步骤
     *  @param stateFunc 非线性状态转移函数 f(x)
     *  @param jacobianF 状态转移Jacobian F = df/dx
     *  @param processNoise 过程噪声协方差Q */
    void predict(const VecFunc& stateFunc,
                 const JacobianFunc& jacobianF,
                 const Matrix& processNoise);

    /** @brief 更新步骤
     *  @param obs 观测向量
     *  @param obsFunc 非线性观测函数 h(x)
     *  @param jacobianH 观测Jacobian H = dh/dx
     *  @param measNoise 测量噪声协方差R */
    void update(const QVector<double>& obs,
                const VecFunc& obsFunc,
                const JacobianFunc& jacobianH,
                const Matrix& measNoise);

    /** @brief 获取当前状态估计 @return 状态向量 */
    QVector<double> state() const { return m_state; }

    /** @brief 获取当前协方差 @return 协方差矩阵 */
    const Matrix& covariance() const { return m_P; }

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 预测完成 @param stateDim 状态维度 */
    void predictionCompleted(int stateDim);

    /** @brief 更新完成 @param stateDim 状态维度 @param innovationNorm 新息范数 */
    void updateCompleted(int stateDim, double innovationNorm);

private:
    /** @brief 矩阵乘法 @param A 矩阵A @param B 矩阵B @return A*B */
    static Matrix matMul(const Matrix& A, const Matrix& B);

    /** @brief 矩阵转置 @param M 矩阵 @return M^T */
    static Matrix matTranspose(const Matrix& M);

    /** @brief 矩阵加法 @param A 矩阵A @param B 矩阵B @return A+B */
    static Matrix matAdd(const Matrix& A, const Matrix& B);

    /** @brief 矩阵求逆(Gauss-Jordan) @param M 矩阵 @return 逆矩阵 */
    static Matrix matInverse(const Matrix& M);

    /** @brief 向量减法 @param a 向量a @param b 向量b @return a-b */
    static QVector<double> vecSub(const QVector<double>& a,
                                  const QVector<double>& b);

    /** @brief 单位矩阵 @param n 维度 @return I_n */
    static Matrix identity(int n);

    QVector<double> m_state;   ///< 状态向量
    Matrix m_P;                 ///< 误差协方差矩阵
    double m_timeSum;           ///< 处理时间累加器
    Stats  m_stats;             ///< 统计信息
};

#endif // EXTENDEDKALMAN_H
