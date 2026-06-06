#include "j16489/m16489.h"
QVector<double> m16489::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
