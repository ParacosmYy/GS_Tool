#include "a32480/m32480.h"
QVector<double> m32480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
