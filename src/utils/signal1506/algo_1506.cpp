/**
 * @file algo_1506.cpp
 * @brief Algorithm module 1506
 */
#include "signal1506/algo_1506.h"
QVector<double> algo_1506::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
