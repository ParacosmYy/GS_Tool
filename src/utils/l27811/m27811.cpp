#include "l27811/m27811.h"
QVector<double> m27811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
