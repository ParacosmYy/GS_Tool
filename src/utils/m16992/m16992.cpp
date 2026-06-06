#include "m16992/m16992.h"
QVector<double> m16992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
