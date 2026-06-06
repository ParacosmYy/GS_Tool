#include "m27032/m27032.h"
QVector<double> m27032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
