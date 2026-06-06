#include "b16921/m16921.h"
QVector<double> m16921::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
