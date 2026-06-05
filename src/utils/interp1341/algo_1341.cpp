/**
 * @file algo_1341.cpp
 * @brief Algorithm module 1341
 */
#include "interp1341/algo_1341.h"
QVector<double> algo_1341::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
