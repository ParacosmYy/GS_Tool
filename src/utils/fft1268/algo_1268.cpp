/**
 * @file algo_1268.cpp
 * @brief Algorithm module 1268
 */
#include "fft1268/algo_1268.h"
QVector<double> algo_1268::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
