#include "c18082/m18082.h"
QVector<double> m18082::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
