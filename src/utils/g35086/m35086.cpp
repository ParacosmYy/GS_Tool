#include "g35086/m35086.h"
QVector<double> m35086::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
