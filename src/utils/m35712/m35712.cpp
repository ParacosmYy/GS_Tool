#include "m35712/m35712.h"
QVector<double> m35712::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
