#include "f20685/m20685.h"
QVector<double> m20685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
