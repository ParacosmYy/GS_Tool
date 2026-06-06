#include "l8511/m8511.h"
QVector<double> m8511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
