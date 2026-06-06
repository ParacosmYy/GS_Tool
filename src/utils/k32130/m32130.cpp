#include "k32130/m32130.h"
QVector<double> m32130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
