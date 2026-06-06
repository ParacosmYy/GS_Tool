#include "m23052/m23052.h"
QVector<double> m23052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
