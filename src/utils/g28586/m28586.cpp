#include "g28586/m28586.h"
QVector<double> m28586::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
