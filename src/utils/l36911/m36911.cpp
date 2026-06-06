#include "l36911/m36911.h"
QVector<double> m36911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
