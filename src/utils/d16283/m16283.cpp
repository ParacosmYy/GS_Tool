#include "d16283/m16283.h"
QVector<double> m16283::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
