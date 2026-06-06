#include "h9507/m9507.h"
QVector<double> m9507::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
