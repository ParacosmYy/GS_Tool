#include "f8685/m8685.h"
QVector<double> m8685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
