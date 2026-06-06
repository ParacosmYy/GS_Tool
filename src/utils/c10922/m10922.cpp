#include "c10922/m10922.h"
QVector<double> m10922::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
