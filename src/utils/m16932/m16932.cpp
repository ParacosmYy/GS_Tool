#include "m16932/m16932.h"
QVector<double> m16932::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
