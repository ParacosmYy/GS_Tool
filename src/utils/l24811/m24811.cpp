#include "l24811/m24811.h"
QVector<double> m24811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
