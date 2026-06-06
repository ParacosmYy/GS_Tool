#include "t9659/m9659.h"
QVector<double> m9659::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
