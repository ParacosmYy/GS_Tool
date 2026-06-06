#include "a32600/m32600.h"
QVector<double> m32600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
