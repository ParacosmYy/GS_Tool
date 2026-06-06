#include "d35723/m35723.h"
QVector<double> m35723::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
