#include "m24152/m24152.h"
QVector<double> m24152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
