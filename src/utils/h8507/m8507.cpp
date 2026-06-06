#include "h8507/m8507.h"
QVector<double> m8507::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
