#include "l8891/m8891.h"
QVector<double> m8891::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
