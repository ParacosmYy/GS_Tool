#include "k10090/m10090.h"
QVector<double> m10090::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
