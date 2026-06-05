/**
 * @file algo_1012.cpp
 * @brief Algorithm module 1012
 */
#include "compress1012/algo_1012.h"
QVector<double> algo_1012::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
