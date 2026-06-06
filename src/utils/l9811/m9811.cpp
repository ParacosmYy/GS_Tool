#include "l9811/m9811.h"
QVector<double> m9811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
