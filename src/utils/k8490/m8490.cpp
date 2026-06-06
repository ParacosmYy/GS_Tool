#include "k8490/m8490.h"
QVector<double> m8490::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
