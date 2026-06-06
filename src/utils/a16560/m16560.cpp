#include "a16560/m16560.h"
QVector<double> m16560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
