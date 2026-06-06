#include "m28912/m28912.h"
QVector<double> m28912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
