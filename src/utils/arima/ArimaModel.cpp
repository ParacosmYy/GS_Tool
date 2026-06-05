#include "utils/arima/ArimaModel.h"
#include <QElapsedTimer>
#include <QtMath>

ArimaModel::ArimaModel(QObject* parent)
    : QObject(parent), m_p(0), m_d(0), m_q(0), m_timeSum(0.0) {}

double ArimaModel::fit(const QVector<double>& series, int p, int d, int q) {
    QElapsedTimer timer; timer.start();
    m_p = p; m_d = d; m_q = q;
    m_original = series;
    m_diffSeries = difference(series, d);
    int n = m_diffSeries.size();
    if (n <= p + q) return 0.0;

    /* 简化: 使用Yule-Walker估计AR参数 */
    m_arCoeffs.resize(p);
    m_maCoeffs.resize(q);
    if (p > 0) {
        QVector<double> autocorr(p + 1);
        for (int lag = 0; lag <= p; ++lag) {
            double sum = 0.0;
            for (int i = lag; i < n; ++i) sum += m_diffSeries[i] * m_diffSeries[i - lag];
            autocorr[lag] = sum / static_cast<double>(n);
        }
        /* Levinson-Durbin */
        if (autocorr[0] > 1e-15) {
            double oldErr = autocorr[0];
            m_arCoeffs[0] = autocorr[1] / oldErr;
            for (int k = 1; k < p; ++k) {
                double num = 0.0;
                for (int j = 0; j < k; ++j) num += m_arCoeffs[j] * autocorr[k - j];
                double ref = (autocorr[k + 1] - num) / oldErr;
                QVector<double> old = m_arCoeffs;
                m_arCoeffs[k] = ref;
                for (int j = 0; j < k; ++j)
                    m_arCoeffs[j] = old[j] - ref * old[k - 1 - j];
                oldErr *= (1.0 - ref * ref);
            }
        }
    }

    /* 计算残差 */
    m_residuals.resize(n);
    for (int i = 0; i < n; ++i) {
        double pred = 0.0;
        for (int j = 0; j < p && i - j - 1 >= 0; ++j)
            pred += m_arCoeffs[j] * m_diffSeries[i - j - 1];
        m_residuals[i] = m_diffSeries[i] - pred;
    }

    /* 残差方差 */
    double var = 0.0;
    for (double r : m_residuals) var += r * r;
    var /= static_cast<double>(n);

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalFits;
    double total = static_cast<double>(m_stats.totalFits + m_stats.totalForecasts);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;
    return var;
}

QVector<double> ArimaModel::forecast(int steps) {
    QVector<double> result(steps, 0.0);
    QVector<double> extended = m_diffSeries;
    for (int s = 0; s < steps; ++s) {
        double pred = 0.0;
        int n = extended.size();
        for (int j = 0; j < m_p && n - j - 1 >= 0; ++j)
            pred += m_arCoeffs[j] * extended[n - j - 1];
        result[s] = pred;
        extended.append(pred);
    }
    /* 逆差分 */
    if (m_d > 0) {
        double lastVal = m_original.last();
        for (int s = 0; s < steps; ++s) {
            result[s] += lastVal;
            lastVal = result[s];
        }
    }
    ++m_stats.totalForecasts;
    emit forecastCompleted(steps);
    return result;
}

QVector<double> ArimaModel::difference(const QVector<double>& s, int d) const {
    QVector<double> result = s;
    for (int i = 0; i < d; ++i) {
        QVector<double> diff;
        for (int j = 1; j < result.size(); ++j)
            diff.append(result[j] - result[j - 1]);
        result = diff;
    }
    return result;
}

void ArimaModel::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
