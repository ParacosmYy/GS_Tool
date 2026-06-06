#include "k18090/m18090.h"
QVector<double> m18090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
