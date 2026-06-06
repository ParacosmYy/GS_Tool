#include "o32754/m32754.h"
QVector<double> m32754::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
