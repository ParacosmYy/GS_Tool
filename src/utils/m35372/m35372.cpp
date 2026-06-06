#include "m35372/m35372.h"
QVector<double> m35372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
