/**
 * @file ZcaWhitening.cpp
 * @brief ZCA白化变换实现
 */

#include "utils/whitening2/ZcaWhitening.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
ZcaWhitening::ZcaWhitening(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 对称矩阵特征分解(Jacobi迭代)
 * @param matrix 对称矩阵(n×n)
 * @param eigenValues 输出特征值(升序)
 * @param eigenVectors 输出特征向量(列存储，n×n)
 * @return 是否成功
 *
 * Jacobi方法通过旋转消去非对角元素，收敛后对角元素即为特征值。
 */
bool ZcaWhitening::eigenDecompose(const QVector<QVector<double>>& matrix,
                                   QVector<double>& eigenValues,
                                   QVector<QVector<double>>& eigenVectors)
{
    int n = matrix.size();
    if (n == 0) return false;

    /* 拷贝矩阵用于迭代 */
    QVector<QVector<double>> A = matrix;
    eigenVectors = QVector<QVector<double>>(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) eigenVectors[i][i] = 1.0;

    int maxIter = 100 * n * n;

    for (int iter = 0; iter < maxIter; ++iter) {
        /* 找最大非对角元素 */
        int p = 0, q = 1;
        double maxVal = std::abs(A[0][1]);
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (std::abs(A[i][j]) > maxVal) {
                    maxVal = std::abs(A[i][j]);
                    p = i;
                    q = j;
                }
            }
        }

        /* 收敛判定 */
        if (maxVal < 1e-12) break;

        /* 计算旋转角度 */
        double app = A[p][p];
        double aqq = A[q][q];
        double apq = A[p][q];

        double theta = 0.0;
        if (std::abs(app - aqq) < 1e-15) {
            theta = M_PI / 4.0;
        } else {
            theta = 0.5 * std::atan2(2.0 * apq, app - aqq);
        }

        double c = std::cos(theta);
        double s = std::sin(theta);

        /* 应用Givens旋转 */
        for (int i = 0; i < n; ++i) {
            if (i == p || i == q) continue;
            double aip = A[i][p];
            double aiq = A[i][q];
            A[i][p] = c * aip + s * aiq;
            A[p][i] = A[i][p];
            A[i][q] = -s * aip + c * aiq;
            A[q][i] = A[i][q];
        }

        double newPP = c * c * app + 2.0 * s * c * apq + s * s * aqq;
        double newQQ = s * s * app - 2.0 * s * c * apq + c * c * aqq;
        A[p][p] = newPP;
        A[q][q] = newQQ;
        A[p][q] = 0.0;
        A[q][p] = 0.0;

        /* 更新特征向量 */
        for (int i = 0; i < n; ++i) {
            double vip = eigenVectors[i][p];
            double viq = eigenVectors[i][q];
            eigenVectors[i][p] = c * vip + s * viq;
            eigenVectors[i][q] = -s * vip + c * viq;
        }
    }

    /* 提取特征值 */
    eigenValues.resize(n);
    for (int i = 0; i < n; ++i) eigenValues[i] = A[i][i];

    return true;
}

/**
 * @brief 拟合白化模型
 * @param data 输入数据矩阵 [n_samples][n_features]
 *
 * 1. 计算各维度均值
 * 2. 计算协方差矩阵
 * 3. 特征分解
 * 4. 构造白化/逆白化矩阵
 */
void ZcaWhitening::fit(const QVector<QVector<double>>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) return;
    int d = data[0].size();
    if (d == 0) return;

    m_dimensions = d;

    /* 第一步: 计算均值 */
    m_mean.resize(d, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < d; ++j) {
            m_mean[j] += data[i][j];
        }
    }
    for (int j = 0; j < d; ++j) {
        m_mean[j] /= static_cast<double>(n);
    }

    /* 第二步: 计算协方差矩阵 */
    QVector<QVector<double>> cov(d, QVector<double>(d, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < d; ++j) {
            for (int k = j; k < d; ++k) {
                double dj = data[i][j] - m_mean[j];
                double dk = data[i][k] - m_mean[k];
                cov[j][k] += dj * dk;
            }
        }
    }
    double invN = 1.0 / static_cast<double>(n);
    for (int j = 0; j < d; ++j) {
        for (int k = j; k < d; ++k) {
            cov[j][k] *= invN;
            cov[k][j] = cov[j][k];
        }
    }

    /* 第三步: 特征分解 */
    QVector<double> eigenValues;
    QVector<QVector<double>> eigenVectors;
    if (!eigenDecompose(cov, eigenValues, eigenVectors)) return;

    /* 第四步: 构造白化矩阵 W = E * D^(-1/2) * E^T */
    /* D^(-1/2): 特征值取逆平方根(加正则化防止除零) */
    QVector<double> invSqrtEigen(d, 0.0);
    for (int i = 0; i < d; ++i) {
        double ev = std::max(eigenValues[i], 1e-8);
        invSqrtEigen[i] = 1.0 / std::sqrt(ev);
    }

    /* W = E * diag(D^(-1/2)) * E^T */
    m_whiteningMatrix = QVector<QVector<double>>(d, QVector<double>(d, 0.0));
    for (int i = 0; i < d; ++i) {
        for (int j = 0; j < d; ++j) {
            double sum = 0.0;
            for (int k = 0; k < d; ++k) {
                sum += eigenVectors[i][k] * invSqrtEigen[k]
                       * eigenVectors[j][k];
            }
            m_whiteningMatrix[i][j] = sum;
        }
    }

    /* W^(-1) = E * diag(D^(1/2)) * E^T */
    m_dewhiteningMatrix = QVector<QVector<double>>(d, QVector<double>(d, 0.0));
    for (int i = 0; i < d; ++i) {
        for (int j = 0; j < d; ++j) {
            double sum = 0.0;
            for (int k = 0; k < d; ++k) {
                double sqrtEv = std::sqrt(std::max(eigenValues[k], 1e-8));
                sum += eigenVectors[i][k] * sqrtEv * eigenVectors[j][k];
            }
            m_dewhiteningMatrix[i][j] = sum;
        }
    }

    m_fitted = true;

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSumMs / qMax(m_stats.totalTransformed, 1ULL);

    emit fitted(d);
}

/**
 * @brief 使用已拟合模型变换数据
 * @param data 输入数据矩阵
 * @return 白化后的数据矩阵
 *
 * y = W * (x - mean)
 */
QVector<QVector<double>> ZcaWhitening::transform(
    const QVector<QVector<double>>& data) const
{
    QElapsedTimer timer;
    timer.start();

    if (!m_fitted || data.isEmpty()) return {};

    int n = data.size();
    int d = m_dimensions;
    QVector<QVector<double>> result(n, QVector<double>(d, 0.0));

    for (int i = 0; i < n; ++i) {
        /* 中心化 */
        QVector<double> centered(d, 0.0);
        for (int j = 0; j < d; ++j) {
            centered[j] = data[i][j] - m_mean[j];
        }
        /* 白化变换 */
        for (int j = 0; j < d; ++j) {
            double sum = 0.0;
            for (int k = 0; k < d; ++k) {
                sum += m_whiteningMatrix[j][k] * centered[k];
            }
            result[i][j] = sum;
        }
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalTransformed;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalTransformed;

    return result;
}

/**
 * @brief 逆变换(从白化空间还原)
 * @param whitened 白化后的数据矩阵
 * @return 还原后的数据矩阵
 *
 * x = W^(-1) * y + mean
 */
QVector<QVector<double>> ZcaWhitening::inverse(
    const QVector<QVector<double>>& whitened) const
{
    QElapsedTimer timer;
    timer.start();

    if (!m_fitted || whitened.isEmpty()) return {};

    int n = whitened.size();
    int d = m_dimensions;
    QVector<QVector<double>> result(n, QVector<double>(d, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < d; ++j) {
            double sum = 0.0;
            for (int k = 0; k < d; ++k) {
                sum += m_dewhiteningMatrix[j][k] * whitened[i][k];
            }
            result[i][j] = sum + m_mean[j];
        }
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalTransformed;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalTransformed;

    return result;
}

/** @brief 重置统计信息 */
void ZcaWhitening::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
