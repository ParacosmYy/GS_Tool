#include "a35080/m35080.h"
QVector<double> m35080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
