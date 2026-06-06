#include "k25670/m25670.h"
QVector<double> m25670::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
