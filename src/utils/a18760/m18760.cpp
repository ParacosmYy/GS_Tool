#include "a18760/m18760.h"
QVector<double> m18760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
