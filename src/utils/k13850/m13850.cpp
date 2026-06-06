#include "k13850/m13850.h"
QVector<double> m13850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
