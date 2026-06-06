#include "a16120/m16120.h"
QVector<double> m16120::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
