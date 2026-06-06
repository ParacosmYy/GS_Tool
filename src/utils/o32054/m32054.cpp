#include "o32054/m32054.h"
QVector<double> m32054::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
