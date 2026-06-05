/**
 * @file algo_1006.cpp
 * @brief Algorithm module 1006
 */
#include "signal1006/algo_1006.h"
QVector<double> algo_1006::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
