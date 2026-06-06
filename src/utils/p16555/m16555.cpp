#include "p16555/m16555.h"
QVector<double> m16555::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
