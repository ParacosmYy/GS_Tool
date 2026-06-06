#include "k30950/m30950.h"
QVector<double> m30950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
