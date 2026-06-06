#include "f9265/m9265.h"
QVector<double> m9265::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
