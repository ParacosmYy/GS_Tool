/**
 * @file algo_1517.cpp
 * @brief Algorithm module 1517
 */
#include "image1517/algo_1517.h"
QVector<double> algo_1517::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
