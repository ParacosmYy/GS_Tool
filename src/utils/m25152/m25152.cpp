#include "m25152/m25152.h"
QVector<double> m25152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
