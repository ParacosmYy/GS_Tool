#include "b16881/m16881.h"
QVector<double> m16881::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
