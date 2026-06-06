#include "a12400/m12400.h"
QVector<double> m12400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
