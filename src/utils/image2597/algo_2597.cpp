/**
 * @file algo_2597.cpp
 * @brief Algorithm module 2597
 */
#include "image2597/algo_2597.h"
QVector<double> algo_2597::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
