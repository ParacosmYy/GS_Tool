#include "i12988/m12988.h"
QVector<double> m12988::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
