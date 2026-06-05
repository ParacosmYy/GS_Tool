/**
 * @file algo_961.cpp
 * @brief Algorithm module 961
 */
#include "interp961/algo_961.h"
QVector<double> algo_961::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
