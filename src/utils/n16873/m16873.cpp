#include "n16873/m16873.h"
QVector<double> m16873::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
