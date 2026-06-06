#include "m32772/m32772.h"
QVector<double> m32772::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
