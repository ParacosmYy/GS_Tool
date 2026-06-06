#include "a12200/m12200.h"
QVector<double> m12200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
