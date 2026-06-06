#include "i9068/m9068.h"
QVector<double> m9068::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
