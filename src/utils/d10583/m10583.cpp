#include "d10583/m10583.h"
QVector<double> m10583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
