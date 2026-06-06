#include "n16573/m16573.h"
QVector<double> m16573::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
