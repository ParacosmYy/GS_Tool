#include "e37004/m37004.h"
QVector<double> m37004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
