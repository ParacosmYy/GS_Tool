#include "c32002/m32002.h"
QVector<double> m32002::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
