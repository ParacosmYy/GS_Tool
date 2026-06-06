#include "m15092/m15092.h"
QVector<double> m15092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
