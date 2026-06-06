#include "m12732/m12732.h"
QVector<double> m12732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
