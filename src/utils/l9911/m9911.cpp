#include "l9911/m9911.h"
QVector<double> m9911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
