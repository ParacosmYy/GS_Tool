#include "i16308/m16308.h"
QVector<double> m16308::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
