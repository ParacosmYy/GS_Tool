#include "a24760/m24760.h"
QVector<double> m24760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
