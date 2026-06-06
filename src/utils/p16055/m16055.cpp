#include "p16055/m16055.h"
QVector<double> m16055::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
