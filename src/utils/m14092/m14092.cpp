#include "m14092/m14092.h"
QVector<double> m14092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
