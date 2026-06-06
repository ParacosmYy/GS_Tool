#include "d16403/m16403.h"
QVector<double> m16403::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
