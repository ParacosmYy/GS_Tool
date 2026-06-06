#include "h35627/m35627.h"
QVector<double> m35627::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
