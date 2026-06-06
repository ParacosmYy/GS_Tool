#include "h8727/m8727.h"
QVector<double> m8727::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
