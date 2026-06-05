/**
 * @file algo_2016.cpp
 * @brief Algorithm module 2016
 */
#include "geometry2016/algo_2016.h"
QVector<double> algo_2016::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
