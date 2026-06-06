#include "a9740/m9740.h"
QVector<double> m9740::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
