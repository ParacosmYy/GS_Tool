/**
 * @file algo_906.cpp
 * @brief Algorithm module 906
 */
#include "signal906/algo_906.h"
QVector<double> algo_906::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
