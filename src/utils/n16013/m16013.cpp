#include "n16013/m16013.h"
QVector<double> m16013::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
