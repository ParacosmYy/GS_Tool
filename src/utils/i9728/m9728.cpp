#include "i9728/m9728.h"
QVector<double> m9728::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
