/**
 * @file algo_2797.cpp
 * @brief Algorithm module 2797
 */
#include "image2797/algo_2797.h"
QVector<double> algo_2797::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
