#include "a7840/m7840.h"
QVector<double> m7840::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
