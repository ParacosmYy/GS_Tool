/**
 * @file algo_1294.cpp
 * @brief Algorithm module 1294
 */
#include "numeric1294/algo_1294.h"
QVector<double> algo_1294::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
