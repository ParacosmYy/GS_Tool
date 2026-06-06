#include "b16001/m16001.h"
QVector<double> m16001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
