/**
 * @file algo_2777.cpp
 * @brief Algorithm module 2777
 */
#include "image2777/algo_2777.h"
QVector<double> algo_2777::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
