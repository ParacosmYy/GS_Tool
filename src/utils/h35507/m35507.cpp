#include "h35507/m35507.h"
QVector<double> m35507::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
