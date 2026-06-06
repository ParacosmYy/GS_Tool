#include "g8186/m8186.h"
QVector<double> m8186::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
