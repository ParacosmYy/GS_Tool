#include "m16832/m16832.h"
QVector<double> m16832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
