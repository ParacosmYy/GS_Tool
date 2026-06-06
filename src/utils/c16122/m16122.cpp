#include "c16122/m16122.h"
QVector<double> m16122::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
