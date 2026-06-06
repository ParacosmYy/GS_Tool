#include "h35007/m35007.h"
QVector<double> m35007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
