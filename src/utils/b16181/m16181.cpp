#include "b16181/m16181.h"
QVector<double> m16181::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
