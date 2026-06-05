/**
 * @file algo_1497.cpp
 * @brief Algorithm module 1497
 */
#include "image1497/algo_1497.h"
QVector<double> algo_1497::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
