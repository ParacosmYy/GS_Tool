#include "m13312/m13312.h"
QVector<double> m13312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
