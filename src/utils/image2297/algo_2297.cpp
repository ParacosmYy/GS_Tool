/**
 * @file algo_2297.cpp
 * @brief Algorithm module 2297
 */
#include "image2297/algo_2297.h"
QVector<double> algo_2297::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
