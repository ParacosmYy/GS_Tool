#include "m27092/m27092.h"
QVector<double> m27092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
