#include "m26312/m26312.h"
QVector<double> m26312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
