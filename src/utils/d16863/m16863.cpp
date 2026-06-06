#include "d16863/m16863.h"
QVector<double> m16863::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
