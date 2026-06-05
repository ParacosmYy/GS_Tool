/**
 * @file algo_2793.cpp
 * @brief Algorithm module 2793
 */
#include "crypto2793/algo_2793.h"
QVector<double> algo_2793::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
