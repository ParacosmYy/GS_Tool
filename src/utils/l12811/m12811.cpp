#include "l12811/m12811.h"
QVector<double> m12811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
