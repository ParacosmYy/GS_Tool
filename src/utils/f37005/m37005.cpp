#include "f37005/m37005.h"
QVector<double> m37005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
