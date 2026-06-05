/**
 * @file algo_1997.cpp
 * @brief Algorithm module 1997
 */
#include "image1997/algo_1997.h"
QVector<double> algo_1997::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
