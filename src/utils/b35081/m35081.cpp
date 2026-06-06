#include "b35081/m35081.h"
QVector<double> m35081::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
