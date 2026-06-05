/**
 * @file algo_1717.cpp
 * @brief Algorithm module 1717
 */
#include "image1717/algo_1717.h"
QVector<double> algo_1717::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
