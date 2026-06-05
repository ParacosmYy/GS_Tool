/**
 * @file algo_1837.cpp
 * @brief Algorithm module 1837
 */
#include "image1837/algo_1837.h"
QVector<double> algo_1837::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
