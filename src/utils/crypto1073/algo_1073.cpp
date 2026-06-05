/**
 * @file algo_1073.cpp
 * @brief Algorithm module 1073
 */
#include "crypto1073/algo_1073.h"
QVector<double> algo_1073::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
