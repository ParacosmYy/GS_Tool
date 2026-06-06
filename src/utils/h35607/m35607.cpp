#include "h35607/m35607.h"
QVector<double> m35607::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
