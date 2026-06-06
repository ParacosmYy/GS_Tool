#include "k18710/m18710.h"
QVector<double> m18710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
