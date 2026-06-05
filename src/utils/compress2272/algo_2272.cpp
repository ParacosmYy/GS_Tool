/**
 * @file algo_2272.cpp
 * @brief Algorithm module 2272
 */
#include "compress2272/algo_2272.h"
QVector<double> algo_2272::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
