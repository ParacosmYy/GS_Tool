#include "e9004/m9004.h"
QVector<double> m9004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
