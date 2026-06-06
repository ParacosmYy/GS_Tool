#include "m8172/m8172.h"
QVector<double> m8172::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
