#include "l28811/m28811.h"
QVector<double> m28811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
