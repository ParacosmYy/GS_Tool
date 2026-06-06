#include "s8818/m8818.h"
QVector<double> m8818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
