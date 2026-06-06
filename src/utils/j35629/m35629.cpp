#include "j35629/m35629.h"
QVector<double> m35629::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
