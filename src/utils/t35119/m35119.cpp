#include "t35119/m35119.h"
QVector<double> m35119::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
