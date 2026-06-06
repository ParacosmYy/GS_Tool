#include "f31685/m31685.h"
QVector<double> m31685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
