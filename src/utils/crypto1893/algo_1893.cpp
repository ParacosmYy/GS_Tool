/**
 * @file algo_1893.cpp
 * @brief Algorithm module 1893
 */
#include "crypto1893/algo_1893.h"
QVector<double> algo_1893::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
