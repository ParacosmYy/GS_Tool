#include "g35806/m35806.h"
QVector<double> m35806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
