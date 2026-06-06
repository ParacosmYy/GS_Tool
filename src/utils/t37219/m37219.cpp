#include "t37219/m37219.h"
QVector<double> m37219::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
