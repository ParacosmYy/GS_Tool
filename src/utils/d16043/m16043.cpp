#include "d16043/m16043.h"
QVector<double> m16043::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
