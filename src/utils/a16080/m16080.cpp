#include "a16080/m16080.h"
QVector<double> m16080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
