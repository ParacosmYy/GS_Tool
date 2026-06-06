#include "m31032/m31032.h"
QVector<double> m31032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
