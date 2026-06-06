#include "l15611/m15611.h"
QVector<double> m15611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
