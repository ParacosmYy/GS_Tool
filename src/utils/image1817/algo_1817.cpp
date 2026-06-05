/**
 * @file algo_1817.cpp
 * @brief Algorithm module 1817
 */
#include "image1817/algo_1817.h"
QVector<double> algo_1817::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
