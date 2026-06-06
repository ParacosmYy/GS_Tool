#include "d17003/m17003.h"
QVector<double> m17003::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
