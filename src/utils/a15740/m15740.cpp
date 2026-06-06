#include "a15740/m15740.h"
QVector<double> m15740::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
