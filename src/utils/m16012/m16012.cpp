#include "m16012/m16012.h"
QVector<double> m16012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
