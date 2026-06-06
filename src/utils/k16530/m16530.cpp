#include "k16530/m16530.h"
QVector<double> m16530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
