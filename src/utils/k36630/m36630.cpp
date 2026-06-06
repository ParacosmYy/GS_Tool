#include "k36630/m36630.h"
QVector<double> m36630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
