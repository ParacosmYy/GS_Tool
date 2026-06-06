#include "b16121/m16121.h"
QVector<double> m16121::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
