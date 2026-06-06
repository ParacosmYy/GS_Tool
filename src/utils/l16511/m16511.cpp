#include "l16511/m16511.h"
QVector<double> m16511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
