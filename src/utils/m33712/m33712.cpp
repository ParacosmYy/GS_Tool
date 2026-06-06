#include "m33712/m33712.h"
QVector<double> m33712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
