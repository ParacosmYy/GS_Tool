/**
 * @file algo_837.cpp
 * @brief Algorithm module 837
 */
#include "image837/algo_837.h"
QVector<double> algo_837::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
