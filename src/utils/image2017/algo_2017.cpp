/**
 * @file algo_2017.cpp
 * @brief Algorithm module 2017
 */
#include "image2017/algo_2017.h"
QVector<double> algo_2017::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
