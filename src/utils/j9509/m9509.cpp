#include "j9509/m9509.h"
QVector<double> m9509::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
