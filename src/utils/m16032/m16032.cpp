#include "m16032/m16032.h"
QVector<double> m16032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
