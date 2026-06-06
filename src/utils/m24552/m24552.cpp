#include "m24552/m24552.h"
QVector<double> m24552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
