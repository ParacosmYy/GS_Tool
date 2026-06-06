#include "c8082/m8082.h"
QVector<double> m8082::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
