#include "l25611/m25611.h"
QVector<double> m25611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
