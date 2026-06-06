#include "i16388/m16388.h"
QVector<double> m16388::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
