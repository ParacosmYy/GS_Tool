#include "h18827/m18827.h"
QVector<double> m18827::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
