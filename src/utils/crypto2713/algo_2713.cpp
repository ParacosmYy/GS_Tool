/**
 * @file algo_2713.cpp
 * @brief Algorithm module 2713
 */
#include "crypto2713/algo_2713.h"
QVector<double> algo_2713::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
