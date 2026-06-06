#include "m18572/m18572.h"
QVector<double> m18572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
