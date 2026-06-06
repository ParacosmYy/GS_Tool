#include "k8070/m8070.h"
QVector<double> m8070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
