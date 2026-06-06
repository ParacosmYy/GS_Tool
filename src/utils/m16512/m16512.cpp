#include "m16512/m16512.h"
QVector<double> m16512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
