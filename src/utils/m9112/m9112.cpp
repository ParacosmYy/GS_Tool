#include "m9112/m9112.h"
QVector<double> m9112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
