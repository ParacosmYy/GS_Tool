#include "h16027/m16027.h"
QVector<double> m16027::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
