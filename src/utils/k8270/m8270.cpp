#include "k8270/m8270.h"
QVector<double> m8270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
