#include "b18881/m18881.h"
QVector<double> m18881::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
