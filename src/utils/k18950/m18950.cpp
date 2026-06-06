#include "k18950/m18950.h"
QVector<double> m18950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
