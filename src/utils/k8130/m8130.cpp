#include "k8130/m8130.h"
QVector<double> m8130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
