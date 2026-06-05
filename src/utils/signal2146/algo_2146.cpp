/**
 * @file algo_2146.cpp
 * @brief Algorithm module 2146
 */
#include "signal2146/algo_2146.h"
QVector<double> algo_2146::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
