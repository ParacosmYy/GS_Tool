#include "m21032/m21032.h"
QVector<double> m21032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
