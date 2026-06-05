/**
 * @file algo_902.cpp
 * @brief Algorithm module 902
 */
#include "poly902/algo_902.h"
QVector<double> algo_902::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
