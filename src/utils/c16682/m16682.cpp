#include "c16682/m16682.h"
QVector<double> m16682::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
