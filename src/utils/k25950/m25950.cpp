#include "k25950/m25950.h"
QVector<double> m25950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
