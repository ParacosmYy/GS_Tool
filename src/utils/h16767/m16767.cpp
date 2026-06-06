#include "h16767/m16767.h"
QVector<double> m16767::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
