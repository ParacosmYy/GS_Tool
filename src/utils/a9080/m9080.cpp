#include "a9080/m9080.h"
QVector<double> m9080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
