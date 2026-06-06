#include "m8912/m8912.h"
QVector<double> m8912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
