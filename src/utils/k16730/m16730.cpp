#include "k16730/m16730.h"
QVector<double> m16730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
