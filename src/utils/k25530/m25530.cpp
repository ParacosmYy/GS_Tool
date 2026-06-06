#include "k25530/m25530.h"
QVector<double> m25530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
