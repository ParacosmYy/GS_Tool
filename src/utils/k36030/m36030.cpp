#include "k36030/m36030.h"
QVector<double> m36030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
