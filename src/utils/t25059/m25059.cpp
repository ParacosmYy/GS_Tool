#include "t25059/m25059.h"
QVector<double> m25059::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
