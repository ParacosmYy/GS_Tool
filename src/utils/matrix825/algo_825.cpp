/**
 * @file algo_825.cpp
 * @brief Algorithm module 825
 */
#include "matrix825/algo_825.h"
QVector<double> algo_825::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
