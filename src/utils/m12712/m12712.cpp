#include "m12712/m12712.h"
QVector<double> m12712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
