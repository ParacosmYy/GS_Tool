#include "d15003/m15003.h"
QVector<double> m15003::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
