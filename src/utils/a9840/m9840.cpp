#include "a9840/m9840.h"
QVector<double> m9840::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
