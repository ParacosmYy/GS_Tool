#include "k35690/m35690.h"
QVector<double> m35690::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
