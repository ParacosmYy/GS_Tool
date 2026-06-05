/**
 * @file algo_2577.cpp
 * @brief Algorithm module 2577
 */
#include "image2577/algo_2577.h"
QVector<double> algo_2577::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
