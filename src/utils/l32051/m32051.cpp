#include "l32051/m32051.h"
QVector<double> m32051::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
