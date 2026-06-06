#include "k35070/m35070.h"
QVector<double> m35070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
