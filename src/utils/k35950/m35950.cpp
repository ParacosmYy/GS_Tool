#include "k35950/m35950.h"
QVector<double> m35950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
