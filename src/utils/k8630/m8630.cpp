#include "k8630/m8630.h"
QVector<double> m8630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
