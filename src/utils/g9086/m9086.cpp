#include "g9086/m9086.h"
QVector<double> m9086::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
