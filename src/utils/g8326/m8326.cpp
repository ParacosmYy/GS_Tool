#include "g8326/m8326.h"
QVector<double> m8326::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
