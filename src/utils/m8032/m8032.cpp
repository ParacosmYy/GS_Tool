#include "m8032/m8032.h"
QVector<double> m8032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
