#include "d16923/m16923.h"
QVector<double> m16923::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
