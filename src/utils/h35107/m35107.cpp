#include "h35107/m35107.h"
QVector<double> m35107::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
