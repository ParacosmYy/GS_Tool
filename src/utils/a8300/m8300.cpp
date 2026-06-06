#include "a8300/m8300.h"
QVector<double> m8300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
