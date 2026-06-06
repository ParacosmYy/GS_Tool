#include "a12500/m12500.h"
QVector<double> m12500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
