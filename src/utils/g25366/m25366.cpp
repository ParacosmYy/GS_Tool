#include "g25366/m25366.h"
QVector<double> m25366::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
