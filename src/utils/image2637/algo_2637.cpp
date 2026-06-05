/**
 * @file algo_2637.cpp
 * @brief Algorithm module 2637
 */
#include "image2637/algo_2637.h"
QVector<double> algo_2637::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
