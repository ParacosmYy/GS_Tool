#include "k18390/m18390.h"
QVector<double> m18390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
