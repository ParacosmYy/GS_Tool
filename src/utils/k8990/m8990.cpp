#include "k8990/m8990.h"
QVector<double> m8990::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
