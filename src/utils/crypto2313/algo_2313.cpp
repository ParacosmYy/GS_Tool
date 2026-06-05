/**
 * @file algo_2313.cpp
 * @brief Algorithm module 2313
 */
#include "crypto2313/algo_2313.h"
QVector<double> algo_2313::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
