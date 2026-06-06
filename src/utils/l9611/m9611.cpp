#include "l9611/m9611.h"
QVector<double> m9611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
