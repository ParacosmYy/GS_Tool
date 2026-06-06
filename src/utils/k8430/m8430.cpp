#include "k8430/m8430.h"
QVector<double> m8430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
