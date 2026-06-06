#include "k19850/m19850.h"
QVector<double> m19850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
