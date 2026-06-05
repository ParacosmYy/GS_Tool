/**
 * @file GaussianProcess.cpp
 * @brief 高斯过程回归实现 — RBF 核 + Cholesky 分解
 */

#include "utils/cluster10/GaussianProcess.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
GaussianProcess::GaussianProcess(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置超参数 @param params RBF 核超参数 */
void GaussianProcess::setHyperParams(const HyperParams& params)
{
    m_params = params;
}

/** @brief 获取当前超参数 */
GaussianProcess::HyperParams GaussianProcess::hyperParams() const
{
    return m_params;
}

/**
 * @brief 拟合训练数据
 * @param xTrain 训练输入
 * @param yTrain 训练输出
 * @return true 拟合成功
 *
 * 计算 K + sigma^2 I 的 Cholesky 分解，并求解 K^{-1} y。
 */
bool GaussianProcess::fit(const QVector<double>& xTrain,
                          const QVector<double>& yTrain)
{
    QElapsedTimer timer;
    timer.start();

    int n = xTrain.size();
    if (n != yTrain.size() || n < 1) {
        return false;
    }

    m_xTrain = xTrain;
    m_yTrain = yTrain;

    /* 构建核矩阵 K(i,j) = k(x_i, x_j) + sigma_n^2 * delta_ij */
    QVector<QVector<double>> K(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            K[i][j] = rbfKernel(xTrain[i], xTrain[j]);
        }
        K[i][i] += m_params.noiseVariance;
    }

    /* Cholesky 分解: K = L * L^T */
    m_L = K;
    if (!cholesky(m_L, n)) {
        return false;
    }

    /* 求解 alpha = K^{-1} y: L * z = y, L^T * alpha = z */
    m_alpha = yTrain;
    solveTriangular(m_L, m_alpha, n);
    /* 第二步: L^T * alpha = z */
    QVector<double> temp = m_alpha;
    for (int i = n - 1; i >= 0; --i) {
        double sum = temp[i];
        for (int j = i + 1; j < n; ++j) {
            sum -= m_L[j][i] * m_alpha[j];
        }
        m_alpha[i] = sum / m_L[i][i];
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalFits;
    m_stats.totalTrainingPoints += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalFits);

    emit fitCompleted(n);
    return true;
}

/**
 * @brief 在新点处预测
 * @param xNew 新输入点
 * @return 预测均值和方差
 */
GaussianProcess::Prediction GaussianProcess::predict(double xNew) const
{
    Prediction pred;
    int n = m_xTrain.size();
    if (n == 0 || m_alpha.size() != n) {
        return pred;
    }

    /* 计算核向量 k* = [k(x*, x_1), ..., k(x*, x_n)] */
    QVector<double> kStar(n);
    for (int i = 0; i < n; ++i) {
        kStar[i] = rbfKernel(xNew, m_xTrain[i]);
    }

    /* 预测均值: f* = k*^T * alpha */
    double mean = 0.0;
    for (int i = 0; i < n; ++i) {
        mean += kStar[i] * m_alpha[i];
    }

    /* 预测方差: k(x*,x*) - k*^T * K^{-1} * k* */
    /* 通过 L^{-1} * k* 求解 */
    QVector<double> v = kStar;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            v[i] -= m_L[i][j] * v[j];
        }
        v[i] /= m_L[i][i];
    }

    double variance = rbfKernel(xNew, xNew);
    for (int i = 0; i < n; ++i) {
        variance -= v[i] * v[i];
    }
    variance = qMax(0.0, variance);

    pred.mean = mean;
    pred.variance = variance;
    pred.stdDev = qSqrt(variance);

    return pred;
}

/**
 * @brief 批量预测
 * @param xNew 新输入点列表
 * @return 预测结果列表
 */
QVector<GaussianProcess::Prediction> GaussianProcess::predictBatch(
    const QVector<double>& xNew) const
{
    QVector<Prediction> results;
    results.reserve(xNew.size());
    for (double x : xNew) {
        results.append(predict(x));
    }

    /* 注意: 不在此处更新统计，predict 已是 const */
    return results;
}

/**
 * @brief 通过边际似然梯度优化超参数
 * @param learningRate 学习率
 * @param iterations 迭代次数
 *
 * 优化目标: 最大化 log p(y|X, theta)。
 * 对长度尺度 l 和信号方差 sigma_f^2 进行梯度上升。
 */
void GaussianProcess::optimizeHyperParams(double learningRate, int iterations)
{
    QElapsedTimer timer;
    timer.start();

    if (m_xTrain.isEmpty()) return;

    int n = m_xTrain.size();

    for (int iter = 0; iter < iterations; ++iter) {
        /* 重新拟合 */
        QVector<QVector<double>> K(n, QVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                K[i][j] = rbfKernel(m_xTrain[i], m_xTrain[j]);
            }
            K[i][i] += m_params.noiseVariance;
        }

        auto L = K;
        if (!cholesky(L, n)) break;

        /* 计算 K^{-1} (通过 Cholesky) */
        QVector<QVector<double>> Kinv(n, QVector<double>(n, 0.0));
        for (int col = 0; col < n; ++col) {
            QVector<double> e(n, 0.0);
            e[col] = 1.0;
            /* L * z = e */
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < i; ++j) {
                    e[i] -= L[i][j] * e[j];
                }
                e[i] /= L[i][i];
            }
            /* L^T * x = z */
            for (int i = n - 1; i >= 0; --i) {
                for (int j = i + 1; j < n; ++j) {
                    e[i] -= L[j][i] * e[j];
                }
                e[i] /= L[i][i];
            }
            for (int i = 0; i < n; ++i) {
                Kinv[i][col] = e[i];
            }
        }

        /* alpha * alpha^T - K^{-1} */
        QVector<QVector<double>> aat(n, QVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                aat[i][j] = m_alpha[i] * m_alpha[j] - Kinv[i][j];
            }
        }

        /* 对长度尺度 l 的梯度 */
        double gradL = 0.0, gradSf = 0.0;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                double d = m_xTrain[i] - m_xTrain[j];
                double d2 = d * d;
                double l2 = m_params.lengthScale * m_params.lengthScale;
                double kVal = rbfKernel(m_xTrain[i], m_xTrain[j]);

                gradL += aat[i][j] * kVal * d2 / (l2 * m_params.lengthScale);
                gradSf += aat[i][j] * kVal / m_params.signalVariance;
            }
        }

        /* 梯度上升 */
        m_params.lengthScale *= qExp(learningRate * gradL);
        m_params.signalVariance *= qExp(learningRate * gradSf);

        /* 边界约束 */
        m_params.lengthScale = qBound(0.01, m_params.lengthScale, 1000.0);
        m_params.signalVariance = qBound(0.001, m_params.signalVariance, 10000.0);

        /* 用新参数重新拟合 */
        fit(m_xTrain, m_yTrain);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;

    emit hyperParamsOptimized(m_params.lengthScale, m_params.signalVariance);
}

/** @brief 重置统计信息 */
void GaussianProcess::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_xTrain.clear();
    m_yTrain.clear();
    m_alpha.clear();
    m_L.clear();
}

/**
 * @brief RBF (径向基函数) 核
 * @param x1 点 1
 * @param x2 点 2
 * @return 核函数值
 */
double GaussianProcess::rbfKernel(double x1, double x2) const
{
    double diff = x1 - x2;
    double l2 = m_params.lengthScale * m_params.lengthScale;
    return m_params.signalVariance * qExp(-0.5 * diff * diff / l2);
}

/**
 * @brief RBF 核对 x1 的偏导数
 * @param x1 点 1
 * @param x2 点 2
 * @return 偏导数值
 */
double GaussianProcess::rbfKernelDx(double x1, double x2) const
{
    double diff = x1 - x2;
    double l2 = m_params.lengthScale * m_params.lengthScale;
    return rbfKernel(x1, x2) * (-diff / l2);
}

/**
 * @brief Cholesky 分解 (就地)
 * @param A 对称正定矩阵 (下三角被覆盖为 L)
 * @param n 矩阵维度
 * @return 是否分解成功
 */
bool GaussianProcess::cholesky(QVector<QVector<double>>& A, int n)
{
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            double sum = A[i][j];
            for (int k = 0; k < j; ++k) {
                sum -= A[i][k] * A[j][k];
            }
            if (i == j) {
                if (sum <= 0.0) return false;
                A[i][j] = qSqrt(sum);
            } else {
                A[i][j] = sum / A[j][j];
            }
        }
    }
    return true;
}

/**
 * @brief 下三角方程求解 L * x = b (就地)
 * @param L 下三角矩阵
 * @param b 右端向量 (被解覆盖)
 * @param n 维度
 */
void GaussianProcess::solveTriangular(const QVector<QVector<double>>& L,
                                      QVector<double>& b, int n)
{
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            b[i] -= L[i][j] * b[j];
        }
        b[i] /= L[i][i];
    }
}
