#include "m20552/m20552.h"
QVector<double> m20552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
