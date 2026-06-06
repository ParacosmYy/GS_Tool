#include "k8770/m8770.h"
QVector<double> m8770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
