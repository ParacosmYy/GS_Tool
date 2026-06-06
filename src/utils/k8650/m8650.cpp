#include "k8650/m8650.h"
QVector<double> m8650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
