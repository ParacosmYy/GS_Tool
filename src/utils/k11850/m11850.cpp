#include "k11850/m11850.h"
QVector<double> m11850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
