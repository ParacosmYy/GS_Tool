#include "b9101/m9101.h"
QVector<double> m9101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
