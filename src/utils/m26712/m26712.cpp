#include "m26712/m26712.h"
QVector<double> m26712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
