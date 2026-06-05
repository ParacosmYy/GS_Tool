/**
 * @file algo_2540.cpp
 * @brief Algorithm module 2540
 */
#include "sort2540/algo_2540.h"
QVector<double> algo_2540::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
