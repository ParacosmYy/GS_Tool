#include "a8400/m8400.h"
QVector<double> m8400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
