#include "d18103/m18103.h"
QVector<double> m18103::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
