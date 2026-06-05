/**
 * @file algo_2701.cpp
 * @brief Algorithm module 2701
 */
#include "interp2701/algo_2701.h"
QVector<double> algo_2701::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
