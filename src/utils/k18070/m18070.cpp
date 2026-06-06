#include "k18070/m18070.h"
QVector<double> m18070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
