#include "p9735/m9735.h"
QVector<double> m9735::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
