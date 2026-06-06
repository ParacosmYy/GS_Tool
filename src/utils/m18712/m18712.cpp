#include "m18712/m18712.h"
QVector<double> m18712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
