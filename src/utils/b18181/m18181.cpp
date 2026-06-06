#include "b18181/m18181.h"
QVector<double> m18181::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
