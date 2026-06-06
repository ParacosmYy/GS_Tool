#include "a8740/m8740.h"
QVector<double> m8740::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
