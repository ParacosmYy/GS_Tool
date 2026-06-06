#include "e16004/m16004.h"
QVector<double> m16004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
