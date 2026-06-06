#include "h16827/m16827.h"
QVector<double> m16827::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
