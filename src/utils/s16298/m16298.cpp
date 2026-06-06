#include "s16298/m16298.h"
QVector<double> m16298::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
