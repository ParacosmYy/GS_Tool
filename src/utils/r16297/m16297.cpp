#include "r16297/m16297.h"
QVector<double> m16297::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
