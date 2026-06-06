#include "l7891/m7891.h"
QVector<double> m7891::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
