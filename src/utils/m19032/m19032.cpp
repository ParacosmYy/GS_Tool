#include "m19032/m19032.h"
QVector<double> m19032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
