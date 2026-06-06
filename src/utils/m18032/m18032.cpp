#include "m18032/m18032.h"
QVector<double> m18032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
