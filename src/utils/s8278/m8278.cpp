#include "s8278/m8278.h"
QVector<double> m8278::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
