#include "r37377/m37377.h"
QVector<double> m37377::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
