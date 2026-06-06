#include "k17090/m17090.h"
QVector<double> m17090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
