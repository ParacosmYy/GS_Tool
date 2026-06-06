#include "t25859/m25859.h"
QVector<double> m25859::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
