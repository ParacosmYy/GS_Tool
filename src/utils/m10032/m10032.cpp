#include "m10032/m10032.h"
QVector<double> m10032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
