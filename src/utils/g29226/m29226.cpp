#include "g29226/m29226.h"
QVector<double> m29226::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
