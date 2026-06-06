#include "i16968/m16968.h"
QVector<double> m16968::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
