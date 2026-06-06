#include "e36004/m36004.h"
QVector<double> m36004::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
