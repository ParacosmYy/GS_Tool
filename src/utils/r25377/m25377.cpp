#include "r25377/m25377.h"
QVector<double> m25377::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
