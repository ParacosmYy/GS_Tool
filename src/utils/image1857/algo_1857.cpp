/**
 * @file algo_1857.cpp
 * @brief Algorithm module 1857
 */
#include "image1857/algo_1857.h"
QVector<double> algo_1857::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
