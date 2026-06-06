#include "r32037/m32037.h"
QVector<double> m32037::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
