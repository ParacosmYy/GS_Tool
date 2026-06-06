#include "k8730/m8730.h"
QVector<double> m8730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
