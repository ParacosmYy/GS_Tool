/**
 * @file algo_2157.cpp
 * @brief Algorithm module 2157
 */
#include "image2157/algo_2157.h"
QVector<double> algo_2157::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
