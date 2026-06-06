#include "k8170/m8170.h"
QVector<double> m8170::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
