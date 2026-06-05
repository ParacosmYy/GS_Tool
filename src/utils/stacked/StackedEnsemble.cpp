/**
 * @file StackedEnsemble.cpp
 * @brief 堆叠集成学习实现
 */

#include "utils/stacked/StackedEnsemble.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
StackedEnsemble::StackedEnsemble(QObject* parent)
    : QObject(parent)
{
}

/** @brief 添加基模型 */
void StackedEnsemble::addModel(const QString& name, PredictFn predictFn)
{
    m_models[name] = predictFn;
}

/** @brief 训练元学习器 */
double StackedEnsemble::train(const QVector<QVector<double>>& data,
                               const QVector<double>& labels)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    int m = m_models.size();

    if (n == 0 || m == 0) return 0.0;

    /* 生成元特征矩阵 */
    QVector<QVector<double>> metaFeatures(n);
    for (int i = 0; i < n; ++i) {
        metaFeatures[i] = this->metaFeatures(data[i]);
    }

    /* 最小二乘法拟合线性元学习器: labels = X * weights + bias */
    /* 正规方程: (X^T X) w = X^T y */
    int d = m + 1; /* 包含偏置项 */

    /* 构建增广矩阵 [meta, 1] */
    QVector<QVector<double>> X(n, QVector<double>(d, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            X[i][j] = metaFeatures[i][j];
        }
        X[i][m] = 1.0; /* 偏置列 */
    }

    /* X^T * X */
    QVector<QVector<double>> XtX(d, QVector<double>(d, 0.0));
    for (int i = 0; i < d; ++i) {
        for (int j = 0; j < d; ++j) {
            for (int k = 0; k < n; ++k) {
                XtX[i][j] += X[k][i] * X[k][j];
            }
        }
    }

    /* 正则化: XtX += lambda * I */
    const double lambda = 1e-4;
    for (int i = 0; i < d; ++i) {
        XtX[i][i] += lambda;
    }

    /* X^T * y */
    QVector<double> Xty(d, 0.0);
    for (int i = 0; i < d; ++i) {
        for (int k = 0; k < n; ++k) {
            Xty[i] += X[k][i] * labels[k];
        }
    }

    /* 高斯消元求解 */
    QVector<double> weights = Xty;
    for (int col = 0; col < d; ++col) {
        /* 部分主元 */
        int maxRow = col;
        for (int row = col + 1; row < d; ++row) {
            if (std::abs(XtX[row][col]) > std::abs(XtX[maxRow][col])) {
                maxRow = row;
            }
        }
        if (maxRow != col) {
            std::swap(XtX[col], XtX[maxRow]);
            std::swap(weights[col], weights[maxRow]);
        }

        if (std::abs(XtX[col][col]) < 1e-15) continue;

        for (int row = col + 1; row < d; ++row) {
            double factor = XtX[row][col] / XtX[col][col];
            for (int j = col; j < d; ++j) {
                XtX[row][j] -= factor * XtX[col][j];
            }
            weights[row] -= factor * weights[col];
        }
    }

    /* 回代 */
    for (int i = d - 1; i >= 0; --i) {
        for (int j = i + 1; j < d; ++j) {
            weights[i] -= XtX[i][j] * weights[j];
        }
        if (std::abs(XtX[i][i]) > 1e-15) {
            weights[i] /= XtX[i][i];
        }
    }

    /* 存储权重 */
    m_metaWeights = weights;
    m_metaBias = (d > m) ? weights[m] : 0.0;

    /* 计算训练误差 */
    double error = 0.0;
    for (int i = 0; i < n; ++i) {
        double pred = predict(data[i]);
        double diff = pred - labels[i];
        error += diff * diff;
    }
    error /= n;

    m_stats.totalTrainings++;
    m_timeSum += timer.elapsed();
    double totalCount = static_cast<double>(m_stats.totalTrainings +
                                             m_stats.totalPredictions);
    m_stats.avgProcessingTimeMs = (totalCount > 0) ? m_timeSum / totalCount : 0.0;

    return error;
}

/** @brief 预测样本 */
double StackedEnsemble::predict(const QVector<double>& sample)
{
    m_stats.totalPredictions++;

    QVector<double> mf = metaFeatures(sample);
    double result = m_metaBias;

    for (int i = 0; i < qMin(mf.size(), m_metaWeights.size()); ++i) {
        result += m_metaWeights[i] * mf[i];
    }

    emit predictionCompleted(result);
    return result;
}

/** @brief 重置统计 */
void StackedEnsemble::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 生成元特征 */
QVector<double> StackedEnsemble::metaFeatures(
    const QVector<double>& sample) const
{
    QVector<double> features;
    features.reserve(m_models.size());
    for (auto it = m_models.constBegin(); it != m_models.constEnd(); ++it) {
        features.append(it.value()(sample));
    }
    return features;
}
