/**
 * @file algo_2437.cpp
 * @brief Algorithm module 2437
 */
#include "image2437/algo_2437.h"
QVector<double> algo_2437::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
