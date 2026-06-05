/**
 * @file algo_962.cpp
 * @brief Algorithm module 962
 */
#include "poly962/algo_962.h"
QVector<double> algo_962::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
