#include "s26458/m26458.h"
QVector<double> m26458::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
