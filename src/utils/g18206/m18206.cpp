#include "g18206/m18206.h"
QVector<double> m18206::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
