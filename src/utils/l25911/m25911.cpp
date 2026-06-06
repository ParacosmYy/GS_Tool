#include "l25911/m25911.h"
QVector<double> m25911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
