#include "m9872/m9872.h"
QVector<double> m9872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
