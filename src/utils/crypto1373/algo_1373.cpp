/**
 * @file algo_1373.cpp
 * @brief Algorithm module 1373
 */
#include "crypto1373/algo_1373.h"
QVector<double> algo_1373::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
