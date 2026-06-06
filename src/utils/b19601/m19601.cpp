#include "b19601/m19601.h"
QVector<double> m19601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
