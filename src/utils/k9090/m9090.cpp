#include "k9090/m9090.h"
QVector<double> m9090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
