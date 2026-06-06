#include "e16324/m16324.h"
QVector<double> m16324::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
