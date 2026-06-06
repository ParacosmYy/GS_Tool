#include "g35366/m35366.h"
QVector<double> m35366::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
