#include "b16061/m16061.h"
QVector<double> m16061::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
