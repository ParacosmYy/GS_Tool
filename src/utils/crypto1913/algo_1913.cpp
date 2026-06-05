/**
 * @file algo_1913.cpp
 * @brief Algorithm module 1913
 */
#include "crypto1913/algo_1913.h"
QVector<double> algo_1913::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
