#include "m17152/m17152.h"
QVector<double> m17152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
