#include "t16219/m16219.h"
QVector<double> m16219::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
