#include "m20912/m20912.h"
QVector<double> m20912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
