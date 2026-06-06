#include "s24818/m24818.h"
QVector<double> m24818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
