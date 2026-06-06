#include "m16572/m16572.h"
QVector<double> m16572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
