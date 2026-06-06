#include "a12380/m12380.h"
QVector<double> m12380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
