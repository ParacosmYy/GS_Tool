#include "i16288/m16288.h"
QVector<double> m16288::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
