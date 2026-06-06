#include "m32312/m32312.h"
QVector<double> m32312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
