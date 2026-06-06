#include "m16732/m16732.h"
QVector<double> m16732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
