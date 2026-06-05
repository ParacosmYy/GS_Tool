/**
 * @file algo_2493.cpp
 * @brief Algorithm module 2493
 */
#include "crypto2493/algo_2493.h"
QVector<double> algo_2493::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
