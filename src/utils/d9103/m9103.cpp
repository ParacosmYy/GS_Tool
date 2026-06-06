#include "d9103/m9103.h"
QVector<double> m9103::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
