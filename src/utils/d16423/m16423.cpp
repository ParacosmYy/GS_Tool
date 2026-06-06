#include "d16423/m16423.h"
QVector<double> m16423::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
