#include "f9685/m9685.h"
QVector<double> m9685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
