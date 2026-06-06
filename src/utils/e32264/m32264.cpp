#include "e32264/m32264.h"
QVector<double> m32264::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
