#include "f16685/m16685.h"
QVector<double> m16685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
