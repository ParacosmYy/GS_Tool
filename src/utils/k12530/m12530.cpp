#include "k12530/m12530.h"
QVector<double> m12530::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
