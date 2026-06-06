#include "d16823/m16823.h"
QVector<double> m16823::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
