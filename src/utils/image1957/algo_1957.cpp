/**
 * @file algo_1957.cpp
 * @brief Algorithm module 1957
 */
#include "image1957/algo_1957.h"
QVector<double> algo_1957::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
