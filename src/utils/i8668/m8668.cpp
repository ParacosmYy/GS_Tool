#include "i8668/m8668.h"
QVector<double> m8668::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
