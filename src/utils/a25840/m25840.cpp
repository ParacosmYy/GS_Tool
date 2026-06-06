#include "a25840/m25840.h"
QVector<double> m25840::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
