#include "l7931/m7931.h"
QVector<double> m7931::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
