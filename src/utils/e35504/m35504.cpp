#include "e35504/m35504.h"
QVector<double> m35504::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
