#include "a16900/m16900.h"
QVector<double> m16900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
