#include "l32711/m32711.h"
QVector<double> m32711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
