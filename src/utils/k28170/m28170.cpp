#include "k28170/m28170.h"
QVector<double> m28170::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
