/**
 * @file algo_1352.cpp
 * @brief Algorithm module 1352
 */
#include "compress1352/algo_1352.h"
QVector<double> algo_1352::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
