#include "d16223/m16223.h"
QVector<double> m16223::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
