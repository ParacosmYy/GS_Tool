#include "l17911/m17911.h"
QVector<double> m17911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
