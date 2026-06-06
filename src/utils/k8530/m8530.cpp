#include "k8530/m8530.h"
QVector<double> m8530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
