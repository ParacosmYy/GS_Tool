#include "n9633/m9633.h"
QVector<double> m9633::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
