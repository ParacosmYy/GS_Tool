/**
 * @file algo_2005.cpp
 * @brief Algorithm module 2005
 */
#include "matrix2005/algo_2005.h"
QVector<double> algo_2005::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
