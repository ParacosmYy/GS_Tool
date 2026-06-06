#include "s25038/m25038.h"
QVector<double> m25038::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
