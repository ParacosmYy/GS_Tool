/**
 * @file algo_1377.cpp
 * @brief Algorithm module 1377
 */
#include "image1377/algo_1377.h"
QVector<double> algo_1377::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
