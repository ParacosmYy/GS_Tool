#include "n16813/m16813.h"
QVector<double> m16813::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
