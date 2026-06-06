#include "k8670/m8670.h"
QVector<double> m8670::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
