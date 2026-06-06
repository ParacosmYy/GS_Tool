#include "d9003/m9003.h"
QVector<double> m9003::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
