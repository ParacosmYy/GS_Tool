#include "l25811/m25811.h"
QVector<double> m25811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
