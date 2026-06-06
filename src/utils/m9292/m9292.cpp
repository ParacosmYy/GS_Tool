#include "m9292/m9292.h"
QVector<double> m9292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
