#include "m16672/m16672.h"
QVector<double> m16672::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
