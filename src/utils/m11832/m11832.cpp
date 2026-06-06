#include "m11832/m11832.h"
QVector<double> m11832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
