/**
 * @file algo_857.cpp
 * @brief Algorithm module 857
 */
#include "image857/algo_857.h"
QVector<double> algo_857::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
