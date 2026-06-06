#include "m35012/m35012.h"
QVector<double> m35012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
