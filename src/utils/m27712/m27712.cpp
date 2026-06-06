#include "m27712/m27712.h"
QVector<double> m27712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
