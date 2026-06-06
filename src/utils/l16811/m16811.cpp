#include "l16811/m16811.h"
QVector<double> m16811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
