#include "a25480/m25480.h"
QVector<double> m25480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
