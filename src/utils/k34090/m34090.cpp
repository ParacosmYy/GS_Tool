#include "k34090/m34090.h"
QVector<double> m34090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
