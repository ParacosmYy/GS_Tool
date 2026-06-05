/**
 * @file algo_2012.cpp
 * @brief Algorithm module 2012
 */
#include "compress2012/algo_2012.h"
QVector<double> algo_2012::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
