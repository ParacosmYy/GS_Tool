#include "k8950/m8950.h"
QVector<double> m8950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
