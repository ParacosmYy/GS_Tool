#include "m32992/m32992.h"
QVector<double> m32992::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
