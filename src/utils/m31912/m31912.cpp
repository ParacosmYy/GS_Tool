#include "m31912/m31912.h"
QVector<double> m31912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
