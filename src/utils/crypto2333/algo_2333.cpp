/**
 * @file algo_2333.cpp
 * @brief Algorithm module 2333
 */
#include "crypto2333/algo_2333.h"
QVector<double> algo_2333::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
