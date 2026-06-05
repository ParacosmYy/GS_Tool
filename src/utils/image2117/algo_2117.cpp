/**
 * @file algo_2117.cpp
 * @brief Algorithm module 2117
 */
#include "image2117/algo_2117.h"
QVector<double> algo_2117::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
