#include "m14012/m14012.h"
QVector<double> m14012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
