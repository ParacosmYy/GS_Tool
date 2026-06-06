#include "s16038/m16038.h"
QVector<double> m16038::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
