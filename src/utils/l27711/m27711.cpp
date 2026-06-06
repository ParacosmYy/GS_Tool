#include "l27711/m27711.h"
QVector<double> m27711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
