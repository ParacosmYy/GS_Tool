#include "h24827/m24827.h"
QVector<double> m24827::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
