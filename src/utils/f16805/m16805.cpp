#include "f16805/m16805.h"
QVector<double> m16805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
