#include "e14004/m14004.h"
QVector<double> m14004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
