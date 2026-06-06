#include "k8710/m8710.h"
QVector<double> m8710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
