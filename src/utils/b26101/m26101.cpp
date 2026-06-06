#include "b26101/m26101.h"
QVector<double> m26101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
