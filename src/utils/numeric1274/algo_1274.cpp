/**
 * @file algo_1274.cpp
 * @brief Algorithm module 1274
 */
#include "numeric1274/algo_1274.h"
QVector<double> algo_1274::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
