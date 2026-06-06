#include "m20312/m20312.h"
QVector<double> m20312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
