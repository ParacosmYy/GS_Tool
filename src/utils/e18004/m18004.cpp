#include "e18004/m18004.h"
QVector<double> m18004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
