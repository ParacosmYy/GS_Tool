#include "a16740/m16740.h"
QVector<double> m16740::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
