#include "i16528/m16528.h"
QVector<double> m16528::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
