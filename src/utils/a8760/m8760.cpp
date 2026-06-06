#include "a8760/m8760.h"
QVector<double> m8760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
