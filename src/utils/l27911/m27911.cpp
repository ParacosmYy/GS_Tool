#include "l27911/m27911.h"
QVector<double> m27911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
