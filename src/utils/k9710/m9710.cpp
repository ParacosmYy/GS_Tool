#include "k9710/m9710.h"
QVector<double> m9710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
