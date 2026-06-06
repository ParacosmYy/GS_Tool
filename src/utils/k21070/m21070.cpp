#include "k21070/m21070.h"
QVector<double> m21070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
