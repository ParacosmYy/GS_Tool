#include "m15572/m15572.h"
QVector<double> m15572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
