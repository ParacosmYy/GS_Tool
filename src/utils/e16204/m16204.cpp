#include "e16204/m16204.h"
QVector<double> m16204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
