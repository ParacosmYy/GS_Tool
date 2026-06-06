#include "i16848/m16848.h"
QVector<double> m16848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
