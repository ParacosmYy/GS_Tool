#include "k8110/m8110.h"
QVector<double> m8110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
