#include "b16741/m16741.h"
QVector<double> m16741::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
