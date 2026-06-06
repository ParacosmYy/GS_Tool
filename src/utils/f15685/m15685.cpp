#include "f15685/m15685.h"
QVector<double> m15685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
