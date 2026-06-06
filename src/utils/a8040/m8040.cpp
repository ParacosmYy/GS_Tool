#include "a8040/m8040.h"
QVector<double> m8040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
