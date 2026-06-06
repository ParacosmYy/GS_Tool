#include "f9945/m9945.h"
QVector<double> m9945::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
