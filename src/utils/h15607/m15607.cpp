#include "h15607/m15607.h"
QVector<double> m15607::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
