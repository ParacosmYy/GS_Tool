#include "f9645/m9645.h"
QVector<double> m9645::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
