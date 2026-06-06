#include "m15032/m15032.h"
QVector<double> m15032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
