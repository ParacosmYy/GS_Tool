#include "k25850/m25850.h"
QVector<double> m25850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
