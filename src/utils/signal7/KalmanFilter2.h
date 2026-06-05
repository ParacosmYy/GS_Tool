/**
 * @file KalmanFilter2.h
 * @brief 扩展卡尔曼滤波(EKF) — 非线性状态估计与雅可比计算
 *
 * 功能:
 *   - 銶态预测: 使用非线性状态转移函数 f(x)
 *   - 协方差预测: P = F*P*F^T + Q
 *   - 测量更新: 使用非线性观测函数 h(x) 和卡尔曼增益
 *   - 雅可比矩阵: 数值差分自动计算
 *   - 支持自定义状态/观测维度
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class KalmanFilter2
 * @brief 扩展卡尔曼滤波器 — 非线性系统状态估计
 *
 * EKF通过一阶泰勒展开线性化非线性系统，使用雅可比矩阵替代
 * 线性系统中的转移矩阵。适用于导航、跟踪、传感器融合等场景。
 */
class KalmanFilter2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalPredicts = 0;          /**< 总预测次数 */
        int totalUpdates = 0;           /**< 总更新次数 */
        int totalJacobians = 0;         /**< 总雅可比计算次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 矩阵类型: 二维 QVector */
    using Matrix = QVector<QVector<double>>;

    /** @brief 非线性状态转移函数: x_k = f(x_{k-1}) */
    using StateFunc = std::function<QVector<double>(const QVector<double>&)>;

    /** @brief 非线性观测函数: z = h(x) */
    using ObsFunc = std::function<QVector<double>(const QVector<double>&)>;

    /** @brief EKF参数 */
    struct EKFParams {
        int stateDim = 0;              /**< 状态维度 */
        int obsDim = 0;                /**< 观测维度 */
        Matrix processNoise;           /**< 过程噪声协方差 Q */
        Matrix measurementNoise;       /**< 测量噪声协方差 R */
        StateFunc stateFunc;           /**< 状态转移函数 */
        ObsFunc obsFunc;               /**< 观测函数 */
    };

    /** @brief 构造函数 */
    explicit KalmanFilter2(QObject* parent = nullptr);

    /**
     * @brief 初始化EKF
     * @param params EKF参数(维度、噪声、函数)
     * @param initialState 初始状态向量
     * @param initialCovariance 初始协方差矩阵
     */
    void initialize(const EKFParams& params,
                    const QVector<double>& initialState,
                    const Matrix& initialCovariance);

    /**
     * @brief 预测步骤(时间更新)
     * @param controlInput 控制输入(可选, 维度=stateDim)
     */
    void predict(const QVector<double>& controlInput = QVector<double>());

    /**
     * @brief 更新步骤(测量更新)
     * @param measurement 观测向量
     */
    void update(const QVector<double>& measurement);

    /**
     * @brief 获取当前状态估计
     * @return 状态向量
     */
    QVector<double> state() const;

    /**
     * @brief 获取当前协方差矩阵
     * @return 协方差矩阵
     */
    Matrix covariance() const;

    /**
     * @brief 计算状态转移雅可比矩阵(数值差分)
     * @param state 当前状态
     * @return 雅可比矩阵 F (stateDim x stateDim)
     */
    Matrix computeStateJacobian(const QVector<double>& state) const;

    /**
     * @brief 计算观测雅可比矩阵(数值差分)
     * @param state 当前状态
     * @return 雅可比矩阵 H (obsDim x stateDim)
     */
    Matrix computeObsJacobian(const QVector<double>& state) const;

    /** @brief 是否已初始化 */
    bool isInitialized() const;

    /** @brief 获取创新向量(最后一次更新) */
    QVector<double> innovation() const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 预测完成 */
    void predicted(const QVector<double>& state);

    /** @brief 更新完成 */
    void updated(const QVector<double>& state, double innovationNorm);

private:
    /** @brief 矩阵乘法 */
    Matrix matMul(const Matrix& A, const Matrix& B) const;

    /** @brief 矩阵转置 */
    Matrix matTranspose(const Matrix& A) const;

    /** @brief 矩阵加法 */
    Matrix matAdd(const Matrix& A, const Matrix& B) const;

    /** @brief 矩阵减法 */
    Matrix matSub(const Matrix& A, const Matrix& B) const;

    /** @brief 矩阵求逆(Gauss-Jordan) */
    Matrix matInverse(const Matrix& A) const;

    /** @brief 单位矩阵 */
    Matrix identity(int n) const;

    /** @brief 向量范数 */
    double vecNorm(const QVector<double>& v) const;

    bool m_initialized = false;       /**< 是否已初始化 */
    int m_stateDim = 0;               /**< 状态维度 */
    int m_obsDim = 0;                 /**< 观测维度 */
    QVector<double> m_state;          /**< 当前状态 */
    Matrix m_covariance;              /**< 当前协方差 */
    Matrix m_processNoise;            /**< 过程噪声 Q */
    Matrix m_measNoise;               /**< 测量噪声 R */
    StateFunc m_stateFunc;            /**< 状态转移函数 */
    ObsFunc m_obsFunc;                /**< 观测函数 */
    QVector<double> m_innovation;     /**< 创新向量 */
    Stats m_stats;                    /**< 统计信息 */
    double m_timeSum = 0.0;           /**< 累计时间 */
};
