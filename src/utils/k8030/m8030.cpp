#include "k8030/m8030.h"
QVector<double> m8030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
