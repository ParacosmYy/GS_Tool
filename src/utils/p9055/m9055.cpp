#include "p9055/m9055.h"
QVector<double> m9055::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
