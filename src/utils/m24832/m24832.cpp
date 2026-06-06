#include "m24832/m24832.h"
QVector<double> m24832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
