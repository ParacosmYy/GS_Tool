#include "o32014/m32014.h"
QVector<double> m32014::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
