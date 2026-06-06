#include "l14911/m14911.h"
QVector<double> m14911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
