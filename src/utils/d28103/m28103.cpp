#include "d28103/m28103.h"
QVector<double> m28103::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
