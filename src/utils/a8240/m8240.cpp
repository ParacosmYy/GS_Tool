#include "a8240/m8240.h"
QVector<double> m8240::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
