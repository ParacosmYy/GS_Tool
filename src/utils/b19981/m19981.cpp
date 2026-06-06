#include "b19981/m19981.h"
QVector<double> m19981::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
