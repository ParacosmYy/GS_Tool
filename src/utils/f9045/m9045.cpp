#include "f9045/m9045.h"
QVector<double> m9045::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
