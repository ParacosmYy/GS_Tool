/**
 * @file algo_981.cpp
 * @brief Algorithm module 981
 */
#include "interp981/algo_981.h"
QVector<double> algo_981::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
