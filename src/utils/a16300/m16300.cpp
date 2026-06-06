#include "a16300/m16300.h"
QVector<double> m16300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
