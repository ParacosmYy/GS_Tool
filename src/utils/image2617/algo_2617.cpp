/**
 * @file algo_2617.cpp
 * @brief Algorithm module 2617
 */
#include "image2617/algo_2617.h"
QVector<double> algo_2617::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
