#include "f8545/m8545.h"
QVector<double> m8545::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
