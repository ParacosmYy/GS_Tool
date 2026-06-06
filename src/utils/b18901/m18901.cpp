#include "b18901/m18901.h"
QVector<double> m18901::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
