#include "l12611/m12611.h"
QVector<double> m12611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
