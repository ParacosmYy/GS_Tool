#include "m16292/m16292.h"
QVector<double> m16292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
