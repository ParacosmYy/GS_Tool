#include "c16862/m16862.h"
QVector<double> m16862::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
