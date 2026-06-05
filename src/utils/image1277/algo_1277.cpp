/**
 * @file algo_1277.cpp
 * @brief Algorithm module 1277
 */
#include "image1277/algo_1277.h"
QVector<double> algo_1277::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
