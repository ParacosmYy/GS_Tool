/**
 * @file algo_2317.cpp
 * @brief Algorithm module 2317
 */
#include "image2317/algo_2317.h"
QVector<double> algo_2317::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
