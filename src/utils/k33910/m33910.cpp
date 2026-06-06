#include "k33910/m33910.h"
QVector<double> m33910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
