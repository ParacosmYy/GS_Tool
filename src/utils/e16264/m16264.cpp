#include "e16264/m16264.h"
QVector<double> m16264::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
