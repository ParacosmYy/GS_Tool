#include "l8811/m8811.h"
QVector<double> m8811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
