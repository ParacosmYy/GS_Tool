#include "m18312/m18312.h"
QVector<double> m18312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
