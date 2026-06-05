/**
 * @file algo_2717.cpp
 * @brief Algorithm module 2717
 */
#include "image2717/algo_2717.h"
QVector<double> algo_2717::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
