#include "f27685/m27685.h"
QVector<double> m27685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
