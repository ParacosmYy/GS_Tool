#include "i9788/m9788.h"
QVector<double> m9788::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
