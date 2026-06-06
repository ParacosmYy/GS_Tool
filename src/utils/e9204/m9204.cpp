#include "e9204/m9204.h"
QVector<double> m9204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
