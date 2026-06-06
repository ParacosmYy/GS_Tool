#include "m37272/m37272.h"
QVector<double> m37272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
