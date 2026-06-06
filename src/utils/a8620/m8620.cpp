#include "a8620/m8620.h"
QVector<double> m8620::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
