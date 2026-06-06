#include "l32611/m32611.h"
QVector<double> m32611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
