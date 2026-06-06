#include "k21130/m21130.h"
QVector<double> m21130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
