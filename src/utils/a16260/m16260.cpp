#include "a16260/m16260.h"
QVector<double> m16260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
