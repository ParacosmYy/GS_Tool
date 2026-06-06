#include "i16628/m16628.h"
QVector<double> m16628::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
