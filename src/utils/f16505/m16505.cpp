#include "f16505/m16505.h"
QVector<double> m16505::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
