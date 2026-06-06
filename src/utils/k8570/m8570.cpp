#include "k8570/m8570.h"
QVector<double> m8570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
