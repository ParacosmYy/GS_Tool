#include "k17710/m17710.h"
QVector<double> m17710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
