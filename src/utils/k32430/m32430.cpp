#include "k32430/m32430.h"
QVector<double> m32430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
