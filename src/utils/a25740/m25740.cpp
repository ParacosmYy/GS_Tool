#include "a25740/m25740.h"
QVector<double> m25740::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
