#include "m36512/m36512.h"
QVector<double> m36512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
