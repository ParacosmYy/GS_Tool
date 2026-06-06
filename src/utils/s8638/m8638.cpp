#include "s8638/m8638.h"
QVector<double> m8638::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
