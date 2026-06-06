#include "a8840/m8840.h"
QVector<double> m8840::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
