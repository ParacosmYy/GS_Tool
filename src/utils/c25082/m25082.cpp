#include "c25082/m25082.h"
QVector<double> m25082::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
