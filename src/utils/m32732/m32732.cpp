#include "m32732/m32732.h"
QVector<double> m32732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
