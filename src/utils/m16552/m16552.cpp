#include "m16552/m16552.h"
QVector<double> m16552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
