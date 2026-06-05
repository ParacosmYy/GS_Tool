/**
 * @file algo_1897.cpp
 * @brief Algorithm module 1897
 */
#include "image1897/algo_1897.h"
QVector<double> algo_1897::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
