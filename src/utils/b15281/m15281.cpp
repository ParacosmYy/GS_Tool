#include "b15281/m15281.h"
QVector<double> m15281::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
