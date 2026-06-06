#include "i16448/m16448.h"
QVector<double> m16448::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
