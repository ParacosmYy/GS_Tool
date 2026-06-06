#include "m13052/m13052.h"
QVector<double> m13052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
