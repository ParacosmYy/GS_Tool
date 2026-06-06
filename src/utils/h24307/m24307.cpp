#include "h24307/m24307.h"
QVector<double> m24307::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
