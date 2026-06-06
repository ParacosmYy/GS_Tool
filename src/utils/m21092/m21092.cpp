#include "m21092/m21092.h"
QVector<double> m21092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
