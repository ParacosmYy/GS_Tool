#include "e28704/m28704.h"
QVector<double> m28704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
