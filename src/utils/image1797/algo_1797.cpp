/**
 * @file algo_1797.cpp
 * @brief Algorithm module 1797
 */
#include "image1797/algo_1797.h"
QVector<double> algo_1797::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
