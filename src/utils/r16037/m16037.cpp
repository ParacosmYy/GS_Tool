#include "r16037/m16037.h"
QVector<double> m16037::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
