#include "k33070/m33070.h"
QVector<double> m33070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
