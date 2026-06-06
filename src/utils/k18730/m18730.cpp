#include "k18730/m18730.h"
QVector<double> m18730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
