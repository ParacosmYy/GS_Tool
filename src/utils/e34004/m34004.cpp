#include "e34004/m34004.h"
QVector<double> m34004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
