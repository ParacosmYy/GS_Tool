#include "s16078/m16078.h"
QVector<double> m16078::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
